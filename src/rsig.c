/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

/*
 * Notes about the design: Most of the things done here is process global,
 * for example the signal actions and masks are not designed to be changed
 * while the program is running. This is because it's impossible to write
 * code that is safe and correctly changes the signal configuration while the
 * program is running. These weaknesses where know when the code was written
 * and stems from general broken-ness in the design (or rather lack of design)
 * in unix when it comes to anything asynchronous and message passing.
 *
 * While there is an interface that allows programs to sets their own signal
 * handlers, doing so is incredibly unsafe and should be avoided. For example,
 * if a signal is accepted in the memory allocator while it's holding a lock
 * the program could deadlock in the prolouge of the signal handler without
 * even entering the function.
 *
 */

#include "rcd.h"
#include "rsig.h"
#include "lwthreads-internal.h"
#include "rsig-internal.h"
#include "atomic.h"
#include "reflect.h"
#include "vm.h"

#include "linux.h"

#pragma librcd

static rsig_full_signal_cfg_t rsig_full_scfg;

/// The default signal mask, initialized by rsig_init().
static sigset_t rsig_default_mask;

const size_t rtsig_sigcancel_exit_offset = offsetof(siginfo_t, __si_fields.__rt.si_sigval.sival_int);

struct segv_rh {
    void* addr;
    size_t len;
    segv_rhandler_t segv_rh;
    void* arg_ptr;
    rbtree_node_t node;
};

/// Memory structure for indexing segv region handlers.
struct {
    rbtree_t index;
    rwspinlock_t rwlock;
} segv_rh_mem = {0};

/// Set to true when first segv region handler is registered.
/// This suppresses the immediate alarm print out in the low level sigsegv handler.
bool rsig_has_segv_rh = false;

/// Built in handler for real time cancellation handling.
void rsig_sigcancel_handler();

/// Built in handler for segmentation failures. This is the low-handler that
/// paves the way for the high-level handler to be safely called even though
/// it has a segmented stack prologue.
void rsig_sigsegv_low_handler();

RBTREE_CMP_FN_DECL(cmp_segv_rh_node, segv_rh_t, node, addr);

void rsig_thread_signal_mask_reset() {
    int r_rt_sigprocmask = rt_sigprocmask(SIG_SETMASK, &rsig_default_mask, 0);
    if (r_rt_sigprocmask == -1)
        RCD_SYSCALL_EXCEPTION(rt_sigprocmask, exception_fatal);
}

static void rsig_init_sys_signal_cfg(rsig_full_signal_cfg_t* rsig_full_scfg) {
    // These core signals are passed through to their default signal handler.
    // Their default behavior is usually what you really want.
    memset(rsig_full_scfg, 0, sizeof(*rsig_full_scfg));
    rsig_full_scfg->sig_cfgs[SIGINT].pass = true;
    rsig_full_scfg->sig_cfgs[SIGCONT].pass = true;
    rsig_full_scfg->sig_cfgs[SIGABRT].pass = true;
    rsig_full_scfg->sig_cfgs[SIGTRAP].pass = true;
    rsig_full_scfg->sig_cfgs[SIGUSR1].pass = true;
    // For reasons that need to be researched, we handle the segmentation failure signal but must not block it.
    rsig_full_scfg->sig_cfgs[SIGSEGV].pass = true;
    rsig_full_scfg->sig_cfgs[SIGSEGV].handler_fn = (void*) rsig_sigsegv_low_handler;
    // We ignore SIGUSR1 so we can raise the signal extemporaneously. Used to notify debuggers about new threads.
    rsig_full_scfg->sig_cfgs[SIGUSR1].ignore = true;
    // For reasons that need to be researched, we handle the cancellation signal but must block it as well.
    rsig_full_scfg->sig_cfgs[LWT_ASYNC_CANCEL_SIGNAL].handler_fn = (void*) rsig_sigcancel_handler;
}

__attribute__((weak))
void rsig_init_user_signal_cfg(rsig_full_signal_cfg_t* signal_cfgs) {}

static void rsig_rt_sigaction(int signum, const struct k_sigaction *act, struct k_sigaction *oldact) {
    int r_rt_sigaction = rt_sigaction(signum, act, oldact);
    if (r_rt_sigaction == -1)
        RCD_SYSCALL_EXCEPTION(rt_sigaction, exception_fatal);
}

void rsig_init() {
    // Initialize the segv rhandler tree.
    rbtree_init(&segv_rh_mem.index, cmp_segv_rh_node);
    // Initialize the full signal configuration.
    rsig_init_sys_signal_cfg(&rsig_full_scfg);
    rsig_init_user_signal_cfg(&rsig_full_scfg);
    // Initialize the default signal mask.
    sigfillset(&rsig_default_mask);
    for (int32_t sig = 1; sig < LENGTHOF(rsig_full_scfg.sig_cfgs); sig++) {
        if (rsig_full_scfg.sig_cfgs[sig].pass)
            sigdelset(&rsig_default_mask, sig);
    }
    // Reset the signal mask now when we have a default.
    rsig_thread_signal_mask_reset();
    // Set up all signal action event handlers.
    struct k_sigaction sa = {0};
    for (int32_t sig = 1; sig < LENGTHOF(rsig_full_scfg.sig_cfgs); sig++) {
        // SIGKILL and SIGSTOP can't have actions.
        if (sig == SIGKILL || sig == SIGSTOP)
            continue;
        // All signal actions should use whatever new stack we set up with sigaltstack().
        rsig_signal_cfg_t sig_cfg = rsig_full_scfg.sig_cfgs[sig];
        sa.flags = SA_ONSTACK;
        // Determine handler and additional flags.
        if (sig_cfg.ignore) {
            sa.handler = SIG_IGN;
        } else if (sig_cfg.handler_fn == 0) {
            sa.handler = SIG_DFL;
        } else {
            sa.handler = sig_cfg.handler_fn;
            sa.flags |= SA_SIGINFO | SA_RESTORER;
        }
        // Initialize signal action now and expect it to be unchanged for the rest of the lifetime of the process.
        rsig_rt_sigaction(sig, &sa, 0);
    }
}

segv_rh_t* rsig_segv_rhandler_set(segv_rhandler_t segv_rh, void* addr, size_t len, void* arg_ptr) {
    segv_rh_t* srh = vm_mmap_reserve(sizeof(segv_rh_t), 0);
    srh->addr = addr;
    srh->len = len;
    srh->segv_rh = segv_rh;
    srh->arg_ptr = arg_ptr;
    rbtree_node_t* node;
    atomic_spinlock_wlock(&segv_rh_mem.rwlock); {
        node = rbtree_insert(&srh->node, &segv_rh_mem.index);
    } atomic_spinlock_uwlock(&segv_rh_mem.rwlock);
    if (node != 0)
        throw("segv region handler address collision", exception_fatal);
    rsig_has_segv_rh = true;
    return srh;
}

void rsig_segv_rhandler_unset(segv_rh_t* srh) {
    atomic_spinlock_wlock(&segv_rh_mem.rwlock); {
        rbtree_remove(&srh->node, &segv_rh_mem.index);
    } atomic_spinlock_uwlock(&segv_rh_mem.rwlock);
    vm_mmap_unreserve(srh, sizeof(segv_rh_t));
}

void rsig_segv_rhandler_resize(segv_rh_t* srh, size_t len) {
    srh->len = len;
}

/// "High level" handler for segmentation failures.
/// We cannot use any features here that touches any locks, depends on atomic lwt execution or a well defined fiber state.
/// This prevents us from using any high level features, like memory allocation, so we will only use the stack and make syscalls.
/// We can still trust the stack since we should be running on a separate signal stack if this is called for a fiber.
void rsig_sigsegv_high_handler(int sig, siginfo_t* si, struct ucontext* uc) {
    // Redirect segmentation failures with registered region handlers.
    void* segv_addr = si->__si_fields.__sigfault.si_addr;
    if (rsig_has_segv_rh) {
        segv_rhandler_t segv_rh = 0;
        void* arg_ptr;
        atomic_spinlock_rlock(&segv_rh_mem.rwlock); {
            segv_rh_t* srh = RBTREE_LOOKUP_KEY_LTE(segv_rh_t, node, &segv_rh_mem.index, segv_addr);
            if (srh != 0) {
                assert(srh->addr <= segv_addr);
                if (segv_addr - srh->addr < srh->len) {
                    segv_rh = srh->segv_rh;
                    arg_ptr = srh->arg_ptr;
                }
            }
        } atomic_spinlock_urlock(&segv_rh_mem.rwlock);
        if (segv_rh != 0)
            return segv_rh(segv_addr, arg_ptr);
    }
    // Only allow first segmentation error through to avoid printout mess.
    static int8_t lock = 0;
    atomic_spinlock_lock(&lock);
    // Declare stack buffers we use.
    fstr_t loc_buf, num_buf, line_buf;
    FSTR_STACK_DECL(loc_buf, 512);
    FSTR_STACK_DECL(num_buf, 32);
    FSTR_STACK_DECL(line_buf, 2048);
    // Print error message header by archaic concatenation.
    void* rip = (void*) uc->uc_mcontext.gregs[REG_RIP];
    fstr_t rip_loc_s = rfl_addr_to_location_inp(loc_buf, rip);
    fstr_t invalid_addr_s = fstr_serial_uint(num_buf, (size_t) segv_addr, 16);
    fstr_t err_buf_tail = line_buf;
    fstr_cpy_over(err_buf_tail, "attempted to access the invalid (or protected) address [0x", &err_buf_tail, 0);
    fstr_cpy_over(err_buf_tail, invalid_addr_s, &err_buf_tail, 0);
    fstr_cpy_over(err_buf_tail, "] @ ", &err_buf_tail, 0);
    fstr_cpy_over(err_buf_tail, rip_loc_s, &err_buf_tail, 0);
    fstr_cpy_over(err_buf_tail, "\n", &err_buf_tail, 0);
    fstr_t header_s = {.str = line_buf.str, .len = err_buf_tail.str - line_buf.str};
    rio_debug(header_s);
    // Walk the stack and print a basic back trace. We only print a depth of 1024 frames.
    void** cur_rbp = (void*) uc->uc_mcontext.gregs[REG_RBP];
    for (size_t i = 0; i < 1024 && cur_rbp != 0; i++) {
        // Get the return address for the current frame.
        void* ret_addr = cur_rbp[1];
        if (ret_addr == 0)
            break;
        // Print backtrace line by archaic concatenation.
        fstr_t ret_loc_s = rfl_addr_to_location_inp(loc_buf, ret_addr - 1);
        fstr_t num_s = fstr_serial_uint(num_buf, i, 10);
        fstr_t bt_buf_tail = line_buf;
        fstr_cpy_over(bt_buf_tail, "#", &bt_buf_tail, 0);
        fstr_cpy_over(bt_buf_tail, num_s, &bt_buf_tail, 0);
        fstr_cpy_over(bt_buf_tail, ": ", &bt_buf_tail, 0);
        fstr_cpy_over(bt_buf_tail, ret_loc_s, &bt_buf_tail, 0);
        fstr_cpy_over(bt_buf_tail, "\n", &bt_buf_tail, 0);
        fstr_t bt_s = {.str = line_buf.str, .len = bt_buf_tail.str - line_buf.str};
        rio_debug(bt_s);
        // Walk to the next frame.
        cur_rbp = (void**) cur_rbp[0];
    }
    rio_debug("\n");
    // Print an inspirational quote to reduce programmer frustration.
    fstr_t quotes[] = {
        " \"There are two ways of constructing a software design: One way is to make it so simple that there are obviously no deficiencies, and the other way is to make it so complicated that there are no obvious deficiencies. The first method is far more difficult.\"\n   - C.A.R. Hoare (British computer scientist, winner of the 1980 Turing Award)",
        " \"If debugging is the process of removing software bugs, then programming must be the process of putting them in.\"\n   - Edsger Dijkstra (Dutch computer scientist, winner of the 1972 Turing Award)",
        " \"Measuring programming progress by lines of code is like measuring aircraft building progress by weight.\"\n   - Bill Gates (co-founder of Microsoft)",
        " \"Nine people can't make a baby in a month.\" (regarding the addition of more programmers to get a project completed faster)\n   - Fred Brooks (American computer scientist, winner of the 1999 Turing Award)",
        " \"Debugging is twice as hard as writing the code in the first place. Therefore, if you write the code as cleverly as possible, you are, by definition, not smart enough to debug it.\"\n   - Brian W. Kernighan (Canadian computer scientist, co-author of \"C programming language\")",
        " \"Always code as if the guy who ends up maintaining your code will be a violent psychopath who knows where you live.\"\n   - Martin Golding",
        " \"C makes it easy to shoot yourself in the foot; C++ makes it harder, but when you do, it blows away your whole leg.\"\n   - Bjarne Stroustrup (Danish computer scientist, developer of the C++ programming language)",
        " \"When debugging, novices insert corrective code; experts remove defective code.\"\n   - Richard Pattis",
        " \"Computer science education cannot make anybody an expert programmer any more than studying brushes and pigment can make somebody an expert painter.\"\n   - Eric S. Raymond (American programmer, open source software advocate, author of \"The Cathedral and the Bazaar\")",
        " \"Most good programmers do programming not because they expect to get paid or get adulation by the public, but because it is fun to program.\"\n   - Linus Torvalds (principal force behind the development of the Linux kernel)",
        " \"Programming today is a race between software engineers striving to build bigger and better idiot-proof programs, and the Universe trying to produce bigger and better idiots. So far, the Universe is winning.\"\n   - Rich Cook",
        " \"Any fool can write code that a computer can understand. Good programmers write code that humans can understand.\"\n   - Martin Fowler (author and speaker on software development)",
        " \"Good code is its own best documentation. As you're about to add a comment, ask yourself, 'How can I improve the code so that this comment isn't needed?'\"\n   - Steve McConnell (author of many software engineering books including \"Code Complete\")",
        " \"The problem with using C++ ... is that there's already a strong tendency in the language to require you to know everything before you can do anything.\"\n   - Larry Wall (developer of the Perl language)",
        " \"People think that computer science is the art of geniuses but the actual reality is the opposite, just many people doing things that build on each other, like a wall of mini stones.\"\n   - Donald Knuth (computer scientist)",
        " \"One of my most productive days was throwing away 1000 lines of code.\"\n   - Ken Thompson (computer scientist, early developer of UNIX OS)",
        " \"Most software today is very much like an Egyptian pyramid with millions of bricks piled on top of each other, with no structural integrity, but just done by brute force and thousands of slaves.\"\n   - Alan Kay (American computer scientist)",
        " \"Before software can be reusable it first has to be usable.\"\n   - Ralph Johnson (computer scientist)",
        " \"Premature optimization is the source of all evil.\"\n   - Donald Knuth",
        " \"[Most managers] may defend the schedule and requirements with passion; but that's their job. It's your job to defend the code with equal passion.\"\n   - Robert Martin (Clean code)",
        " \"The first 90% of the code accounts for the first 90% of the development time. The remaining 10% of the code accounts for the other 90% of the development time.\"\n   - Tom Cargil (Bell labs)",
        " \"The best way to predict the future is to implement it.\"\n   - Alan Key",
        " \"Talk is cheap. Show me the code.\"\n   - Linus Torvalds (principal force behind the development of the Linux kernel)",
        " \"Perl – The only language that looks the same before and after RSA encryption.\"\n   - Keith Bostic",
        " \"Controlling complexity is the essence of computer programming.\"\n   - Brian Kernighan",
    };
    rio_debug(quotes[((size_t) rip) % (LENGTHOF(quotes))]);
    // Exit with failure status.
    rio_debug("\n\nprogram cannot recover from segmentation failure, exiting now\n");
}
