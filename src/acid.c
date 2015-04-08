#include "rcd.h"
#include "atomic.h"
#include "linux.h"
#include "rsig-internal.h"
#include "polarssl/sha1.h"
#include "rsig.h"
#include "acid.h"
#include "vm-internal.h"
#include "lwthreads-internal.h"

#pragma librcd

typedef enum sync_op {
    sync_op_none,
    sync_op_commit,
    sync_op_quit
} sync_op_t;

#define VM_SYNC_THREAD_STACK_SIZE (40 * PAGE_SIZE)

/// How many powers of two a page size is.
#define PAGE_2E (12)

CASSERT((1 << PAGE_2E) == PAGE_SIZE);

/// Magic number for acid journals.
const static uint64_t acid_journ_magic = 0xaa2ac040aeac5945;

/// A changed page tracked by the acid layer.
typedef struct acid_page {
    /// Page ID. Number of pages from page 0.
    uint64_t page_id;
    /// When true the kernel has buffered a copy of this page in the journal
    /// i/o write queue so no new page cow is required.
    bool jbuffered;
    /// When 0 the page has been copied to this address while waiting for
    /// the kernel to buffer a copy of it in the journal i/o write queue.
    void* new_page_cow;
    /// Lock for this page.
    int8_t plock;
    rbtree_node_t node;
} acid_page_t;

struct acid {
    /// Sync control.
    struct {
        /// Sync control lock.
        int8_t lock;
        /// Is set to 1 on pending op order.
        int32_t op_futex;
        /// Control operation.
        sync_op_t op;
        /// Is set to 1 on exit complete.
        int32_t exit_futex;
        /// True if sync thread is in sync phase.
        bool in_sync_phase;
        /// Fiber that is created when changing op to commit
        /// and cancelled when journal fsync is complete.
        rcd_fid_t commit_wait_fid;
        /// Fiber that is created when changing op to commit
        /// and cancelled when snapshot is complete.
        rcd_fid_t snapshot_wait_fid;
    } ctrl;
    /// File descriptor for data file.
    int32_t fd_data;
    /// File descriptor for journal file.
    int32_t fd_journal;
    /// Current journaled revision of the database. This is the revision that
    /// is guaranteed to at least have been synced to the journal and will
    /// persist even during sudden termination.
    size_t commit_rev;
    /// Address range for mapping.
    void* base_addr;
    size_t data_length;
    /// Segv region handler for mapping.
    segv_rh_t* srh;
    /// Tree of dirty pages that has not been synced yet.
    rbtree_t dpage_index;
    size_t n_dpage_index;
    /// Tree of new pages that are being synced to the journal.
    rbtree_t npage_index;
    size_t n_npage_index;
    /// Process that syncs journal.
    int32_t sync_pid;
    struct lwt_physical_thread* sync_physt;
    void* sync_stack;
    vm_heap_t* sync_heap;
    /// Lock for page indexes.
    int8_t ilock;
};

typedef struct __attribute__((__packed__)) jrnl_idx {
    /// SHA-1 hash of journal state, not including the hash itself.
    uint8_t sha1_hash[20];
    /// Magic number. Used for simple pre-check if journal has been written
    /// without bothering spending any valuable time on hashing.
    uint64_t magic;
    /// Commit number. How many times the journal has been flushed.
    uint64_t commit_rev;
    /// Last address range of mapping.
    uint64_t data_addr;
    /// Offset of the first data page in journal from start of file in whole
    /// pages. Must be at least 1 since the journal index occupies at least
    /// one page.
    uint64_t page0_offs;
    /// Number of data pages in journal. Length of page_idx.
    uint64_t n_pages;
    /// Data page index. Each integer is the page id for the data page at that
    /// offset in the journal.
    uint64_t page_idx[];
} jrnl_idx_t;

RBTREE_CMP_FN_DECL(cmp_acid_page, acid_page_t, node, page_id)

VM_DEFINE_FREE_LIST_ALLOCATOR_FN(acid_page_t, alloc_acid_page, free_acid_page, true)

static void strict_close(int32_t fd) {
    int32_t close_r = close(fd);
    if (close_r == -1)
        RCD_SYSCALL_EXCEPTION(close, exception_fatal);
}

static void strict_pwrite(int32_t fd, void* buf, size_t count, size_t offset) {
    restart:;
    ssize_t pwrite_r = pwrite(fd, buf, count, offset);
    if (pwrite_r == -1) {
        if (errno == EINTR)
            goto restart;
        RCD_SYSCALL_EXCEPTION(pwrite, exception_fatal);
    }
    if (pwrite_r != count)
        throw("unexpected partial write", exception_fatal);
}

static void strict_fsync(int32_t fd) {
    int32_t fsync_r = fsync(fd);
    if (fsync_r == -1)
        RCD_SYSCALL_EXCEPTION(fsync, exception_fatal);
}

static void strict_ftruncate(int32_t fd, size_t new_size) {
    int32_t ftruncate_r = ftruncate(fd, new_size);
    if (ftruncate_r == -1)
        RCD_SYSCALL_EXCEPTION(ftruncate, exception_fatal);
}

static struct stat strict_fstat(int32_t fd) {
    struct stat s;
    int32_t fstat_r = fstat(fd, &s);
    if (fstat_r == -1)
        RCD_SYSCALL_EXCEPTION(fstat, exception_fatal);
    return s;
}

static void* strict_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void* mmap_r = mmap(addr, length, prot, flags, fd, offset);
    if (mmap_r == MAP_FAILED)
        RCD_SYSCALL_EXCEPTION(mmap, exception_fatal);
    if (addr != 0 && mmap_r != addr)
        throw(concs("mmap() refused to map the requested address [", addr, "], got [", mmap_r, "] instead"), exception_fatal);
    return mmap_r;
}

static void strict_munmap(void* addr, size_t length) {
    int32_t munmap_r = munmap(addr, length);
    if (munmap_r == -1)
        RCD_SYSCALL_EXCEPTION(munmap, exception_fatal);
}

static void strict_mprotect(void* addr, size_t length, int prot) {
    //x-dbg/ DBGFN("strict_mprotect(", addr, ", ", length, ", ", prot, ")");
    int32_t mprotect_r = mprotect(addr, length, prot);
    if (mprotect_r == -1)
        RCD_SYSCALL_EXCEPTION(mprotect, exception_fatal);
}

static void strict_futex(int* uaddr, int op, int val, const struct timespec* timeout, int* uaddr2, int val3) {
    int32_t futex_r = futex(uaddr, op, val, timeout, uaddr2, val3);
    if (futex_r == -1) {
        // Check expected test-and-wait race.
        if (op == FUTEX_WAIT && errno == EWOULDBLOCK)
            return;
        RCD_SYSCALL_EXCEPTION(futex, exception_fatal);
    }
}

static void acid_page_iter_sanity_check(rbtree_node_t** node, size_t* i, size_t n_tree) {
    if (*node == 0) {
        if (*i != n_tree)
            throw("iteration ended on invalid node count, tree is corrupt", exception_fatal);
    } else {
        if (*i >= n_tree)
            throw("iterated beyond node count, tree is corrupt", exception_fatal);
    }
}

static inline acid_page_t* acid_page_first(rbtree_t* tree, rbtree_node_t** node, size_t* i, size_t n_tree) {
    *node = rbtree_first(tree);
    *i = 0;
    acid_page_iter_sanity_check(node, i, n_tree);
    return RBTREE_NODE2ELEM(acid_page_t, node, *node);
}

static inline acid_page_t* acid_page_iter(rbtree_node_t** node, size_t* i, size_t n_tree) {
    *node = rbtree_next(*node);
    *i = *i + 1;
    acid_page_iter_sanity_check(node, i, n_tree);
    return RBTREE_NODE2ELEM(acid_page_t, node, *node);
}

#define foreach_acid_page(tree, n_tree, page) \
    LET(rbtree_node_t* node) \
    LET(size_t i) \
    for (acid_page_t* page = acid_page_first(tree, &node, &i, n_tree); page != 0; page = acid_page_iter(&node, &i, n_tree))

void acid_close(acid_h* ah) {
    if (ah->sync_pid != 0) {
        // Sync any currently uncommited changes or wait for pending commit.
        acid_fsync(ah);
        // Order the sync tread to stop.
        atomic_spinlock_lock(&ah->ctrl.lock); {
            switch (ah->ctrl.op) {{
            } case sync_op_none: {
                ah->ctrl.op = sync_op_quit;
                break;
            } case sync_op_commit: {
            } case sync_op_quit: {
                throw("undefined behavior: closing acid handle while it's in use", exception_fatal);
            }}
        } atomic_spinlock_unlock(&ah->ctrl.lock);
        // Wake sync thread so it can stop itself.
        ah->ctrl.op_futex = 1;
        strict_futex(&ah->ctrl.op_futex, FUTEX_WAKE, INT32_MAX, 0, 0, 0);
        // Wait for sync user thread to be prepared for exit.
        while (ah->ctrl.exit_futex == 0)
            strict_futex(&ah->ctrl.exit_futex, FUTEX_WAIT, 0, 0, 0, 0);
        // Wait for the sync thread to be completely free'd in the kernel.
        lwt_thread_cancel_sync(ah->sync_pid);
    }
    // Free sync thread heap & stack.
    if (ah->sync_heap != 0)
        vm_heap_release(ah->sync_heap, 0, 0);
    if (ah->sync_stack != 0)
        vm_mmap_unreserve(ah->sync_stack, VM_SYNC_THREAD_STACK_SIZE);
    // The indexes should be completely empty or UB has taken place and we
    // should crash the program.
    if (ah->n_dpage_index != 0 || ah->n_npage_index != 0)
        throw("undefined behavior: mutating acid memory while closing it", exception_fatal);
    // Unregister segv region handler.
    if (ah->srh != 0)
        rsig_segv_rhandler_unset(ah->srh);
    // Unmap data memory.
    if (ah->base_addr != 0 && ah->base_addr != MAP_FAILED)
        strict_munmap(ah->base_addr, ah->data_length);
    // Close file descriptors.
    if (ah->fd_journal != -1)
        strict_close(ah->fd_journal);
    if (ah->fd_data != -1)
        strict_close(ah->fd_data);
    // Unreserve handle memory.
    vm_mmap_unreserve(ah, sizeof(acid_h));
}

/// Commits all pages in journal to the data file without syncing.
static void journ_commit(int32_t fd_data, void* journ_ptr) {
    jrnl_idx_t* jidx = journ_ptr;
    void* joffs = journ_ptr + jidx->page0_offs * PAGE_SIZE;
    for (size_t i = 0; i < jidx->n_pages; i++, joffs += PAGE_SIZE) {
        uint64_t page_id = jidx->page_idx[i];
        strict_pwrite(fd_data, joffs, PAGE_SIZE, page_id * PAGE_SIZE);
    }
}

static inline void get_jidx_size(size_t n_data_pages, size_t* out_raw_size, size_t* out_n_pages) {
    *out_raw_size = sizeof(jrnl_idx_t) + sizeof(size_t) * n_data_pages;
    *out_n_pages = (*out_raw_size + PAGE_SIZE - 1) / PAGE_SIZE;
}

/// Returns true if the journal contains non-corrupt changes that have not yet
/// been comitted to the data file.
static bool journ_verify(void* journ_map, size_t total_jsize, size_t data_size) {
    jrnl_idx_t* jidx = journ_map;
    // Verify journal magic.
    if (jidx->magic != acid_journ_magic)
        return false;
    // Calculate and check journal index size.
    size_t raw_jidx_size, n_jidx_pages;
    get_jidx_size(jidx->n_pages, &raw_jidx_size, &n_jidx_pages);
    if (jidx->page0_offs != n_jidx_pages)
        return false;
    // Calculate and check total journal size.
    size_t expect_jsize = (n_jidx_pages + jidx->n_pages) * PAGE_SIZE;
    if (total_jsize != expect_jsize)
        return false;
    // Verify that page index is in-bounds.
    assert(data_size >= PAGE_SIZE);
    size_t last_page_id = (data_size / PAGE_SIZE) - 1;
    for (size_t i = 0; i < jidx->n_pages; i++) {
        if (jidx->page_idx[i] > last_page_id)
            return false;
    }
    // Hash journal and verify it.
    sha1_context hash_ctx;
    sha1_starts(&hash_ctx);
    sha1_update(&hash_ctx, journ_map + jidx->page0_offs * PAGE_SIZE, jidx->n_pages * PAGE_SIZE);
    // Hash the journal, excluding the hash.
    sha1_update(&hash_ctx, journ_map + sizeof(jidx->sha1_hash), raw_jidx_size - sizeof(jidx->sha1_hash));
    uint8_t actual_hash[20];
    sha1_finish(&hash_ctx, actual_hash);
    if (memcmp(actual_hash, jidx->sha1_hash, 20) != 0)
        return false;
    // Journal is a-ok.
    return true;
}

static void acid_sync_commit(acid_h* ah) {
    bool has_changes;
    rcd_fid_t commit_wait_fid, snapshot_wait_fid;
    // Take snapshot and mark all dirty pages as new pages that should be
    // included in the next commit/journal revision.
    // We don't win any throughput in the long run by allowing this snapshot
    // to be taken concurrently while committing the journal to the data file
    // as we would only get less dirty pages to be included in the next commit.
    atomic_spinlock_lock(&ah->ilock); {
        has_changes = (ah->dpage_index.root != 0);
        if (has_changes) {
            // Protect all pages, preventing changes caused by the MMU.
            // This is a red-black tree operation that could take some time.
            int32_t mprotect_r = mprotect(ah->base_addr, ah->data_length, PROT_READ);
            if (mprotect_r == -1)
                RCD_SYSCALL_EXCEPTION(mprotect, exception_fatal);
            // At this point we have effectively taken a snapshot.
            // Index all dirty pages as new pages and reset the dirty page index.
            ah->npage_index = ah->dpage_index;
            ah->n_npage_index = ah->n_dpage_index;
            rbtree_init(&ah->dpage_index, cmp_acid_page);
            ah->n_dpage_index = 0;
        }
        // We now enter the "in sync" phase that lasts until both journal and
        // data sync is complete. Any fsync that begins after this point
        // (when ilock is released) must wait for a new wait fiber as we cannot
        // guarantee that all changes before that fsync is really included in
        // this commit. We therefore move the commit_wait_fid over to the
        // local stack so we hide it and can cancel it later.
        atomic_spinlock_lock(&ah->ctrl.lock); {
            // Reset commit operation so new fsyncs trigger a new commit.
            if (ah->ctrl.op == sync_op_commit)
                ah->ctrl.op = sync_op_none;
            // Trigger in sync phase.
            ah->ctrl.in_sync_phase = has_changes;
            // Consume (possibly zero) commit wait fids.
            commit_wait_fid = ah->ctrl.commit_wait_fid;
            ah->ctrl.commit_wait_fid = 0;
            snapshot_wait_fid = ah->ctrl.snapshot_wait_fid;
            ah->ctrl.snapshot_wait_fid = 0;
        } atomic_spinlock_unlock(&ah->ctrl.lock);
    } atomic_spinlock_unlock(&ah->ilock);
    // Snapshot taken, wake up waiters.
    lwt_cancel_fiber_id(snapshot_wait_fid);
    // When we have no changes, just wake up pending fsync fiber and return.
    if (!has_changes) {
        lwt_cancel_fiber_id(commit_wait_fid);
        return;
    }
    // Read new page index.
    rbtree_t npage_index = ah->npage_index;
    size_t n_npage_index = ah->n_npage_index;
    // Calculate # pages required for journal index and initialize it.
    size_t raw_jidx_size, n_jidx_pages;
    get_jidx_size(n_npage_index, &raw_jidx_size, &n_jidx_pages);
    assert(n_jidx_pages > 0);
    size_t jidx_size = n_jidx_pages * PAGE_SIZE;
    void* jidx_pages = vm_mmap_reserve(jidx_size, 0);
    memset(jidx_pages, 0, jidx_size);
    jrnl_idx_t* jidx = jidx_pages;
    jidx->magic = acid_journ_magic;
    jidx->commit_rev = ah->commit_rev;
    jidx->data_addr = (uint64_t) ah->base_addr;
    jidx->page0_offs = n_jidx_pages;
    jidx->n_pages = n_npage_index;
    // Truncate journal to the required size.
    size_t total_jsize = (n_jidx_pages + n_npage_index) * PAGE_SIZE;
    strict_ftruncate(ah->fd_journal, total_jsize);
    // Start streaming hash.
    sha1_context hash_ctx;
    sha1_starts(&hash_ctx);
    // Index, hash and write all pages.
    // We put them in a vector so we can quickly free
    size_t npage_vec_size = sizeof(acid_page_t*) * n_npage_index;
    acid_page_t** npage_vec = vm_mmap_reserve(npage_vec_size, 0);
    foreach_acid_page(&npage_index, n_npage_index, page) {
        // Index page in vector.
        npage_vec[i] = page;
        // Index page id in journal index.
        jidx->page_idx[i] = page->page_id;
        // Calculate offset in journal where page should be written.
        size_t joffs = jidx->page0_offs * PAGE_SIZE + i * PAGE_SIZE;
        // Outer unsafe check for optimization.
        if (page->new_page_cow != 0) page_cowd: {
            // Page has been cow'd, no need for locks.
            sha1_update(&hash_ctx, page->new_page_cow, PAGE_SIZE);
            strict_pwrite(ah->fd_journal, page->new_page_cow, PAGE_SIZE, joffs);
        } else {
            // Take lock and write page while uncopied.
            atomic_spinlock_lock(&page->plock); {
                // Catch test and set race.
                if (page->new_page_cow != 0) {
                    atomic_spinlock_unlock(&page->plock);
                    goto page_cowd;
                }
                // Inner protected write of pure page.
                void* page_addr = ah->base_addr + page->page_id * PAGE_SIZE;
                sha1_update(&hash_ctx, page_addr, PAGE_SIZE);
                strict_pwrite(ah->fd_journal, page_addr, PAGE_SIZE, joffs);
                page->jbuffered = true;
            } atomic_spinlock_unlock(&page->plock);
        }
    }
    // Write journal index and deallocate it.
    sha1_update(&hash_ctx, jidx_pages + sizeof(jidx->sha1_hash), raw_jidx_size - sizeof(jidx->sha1_hash));
    sha1_finish(&hash_ctx, jidx->sha1_hash);
    strict_pwrite(ah->fd_journal, jidx_pages, raw_jidx_size, 0);
    vm_mmap_unreserve(jidx_pages, jidx_size);
    // Memory map the journal.
    void* journ_map = strict_mmap(0, total_jsize, PROT_READ, MAP_SHARED, ah->fd_journal, 0);
    // Sync the journal.
    strict_fsync(ah->fd_journal);
    // The changes are now persistent.
    // Increment journal rev and detach the npage index.
    atomic_spinlock_lock(&ah->ilock); {
        ah->commit_rev++;
        rbtree_init(&ah->npage_index, cmp_acid_page);
        ah->n_npage_index = 0;
    } atomic_spinlock_unlock(&ah->ilock);
    // Journal synchronized, wake up waiters.
    lwt_cancel_fiber_id(commit_wait_fid);
    // Asynchronously commit the journal to the data file.
    assert(journ_verify(journ_map, total_jsize, ah->data_length));
    journ_commit(ah->fd_data, journ_map);
    // Unmap the journal.
    strict_munmap(journ_map, total_jsize);
    // Free the npage index.
    for (size_t i = 0; i < n_npage_index; i++) {
        acid_page_t* page = npage_vec[i];
        if (page->new_page_cow != 0)
            vm_mmap_unreserve(page->new_page_cow, PAGE_SIZE);
        free_acid_page(page);
    }
    vm_mmap_unreserve(npage_vec, npage_vec_size);
    // Wait for data file to sync.
    strict_fsync(ah->fd_data);
    // Truncate the journal to throw away all pages and only preserve the journal header.
    strict_ftruncate(ah->fd_journal, PAGE_SIZE);
    // In-sync phase ends here. It is now safe to wait for snapshot without
    // waiting for disk sync.
    ah->ctrl.in_sync_phase = false;
}

static void acid_sync_thread(void* arg_ptr) {
    acid_h* ah = arg_ptr;
    lwt_setup_basic_thread(ah->sync_heap, "acid_sync_thread");
    for (;;) {
        // Consume sync operation.
        sync_op_t op;
        atomic_spinlock_lock(&ah->ctrl.lock); {
            op = ah->ctrl.op;
            ah->ctrl.op_futex = 0;
        } atomic_spinlock_unlock(&ah->ctrl.lock);
        // Execute sync operation.
        switch (op) {{
        } case sync_op_none: {
            strict_futex(&ah->ctrl.op_futex, FUTEX_WAIT, 0, 0, 0, 0);
            break;
        } case sync_op_commit: {
            acid_sync_commit(ah);
            break;
        } case sync_op_quit: {
            ah->ctrl.exit_futex = 1;
            strict_futex(&ah->ctrl.exit_futex, FUTEX_WAKE, INT32_MAX, 0, 0, 0);
            _exit(0);
        }}
    }
}

static size_t addr_page_id(acid_h* ah, void* addr) {
    if (addr < ah->base_addr)
        throw(concs("address below acid memory range [", addr, "] < [", ah->base_addr, "]"), exception_fatal);
    if (addr >= ah->base_addr + ah->data_length)
        throw(concs("address above acid memory range [", addr, "] >= [", ah->base_addr, "] + [", ah->data_length, "]"), exception_fatal);
    return ((size_t) (addr - ah->base_addr)) >> PAGE_2E;
}

static void acid_rsig_segv(void* addr, void* arg_ptr) {
    acid_h* ah = arg_ptr;
    //x-dbg/ DBGFN(addr);
    // Got segmentation failure when writing to page in range, unlock page and add it as dirty.
    size_t page_id = addr_page_id(ah, addr);
    void* page_addr = (void*) ((uintptr_t) addr & ~0xfff);
    // We immediately take a write lock. It's not worth making a read locked
    // test first since we expect to almost always need to mutate the index.
    atomic_spinlock_lock(&ah->ilock); {
        // Look up page in new page index and execute cow copy if required before we allow page to mutate.
        acid_page_t* npage = RBTREE_LOOKUP_KEY(acid_page_t, node, &ah->npage_index, page_id);
        if (npage != 0) {
            atomic_spinlock_lock(&npage->plock); {
                if (!npage->jbuffered && npage->new_page_cow == 0) {
                    npage->new_page_cow = vm_mmap_reserve(PAGE_SIZE, 0);
                    memcpy(npage->new_page_cow, page_addr, PAGE_SIZE);
                }
            } atomic_spinlock_unlock(&npage->plock);
        }
        // Look up page in dirty page index.
        acid_page_t* dpage = RBTREE_LOOKUP_KEY(acid_page_t, node, &ah->dpage_index, page_id);
        if (dpage == 0) {
            // Page not already indexed, index it as dirty.
            dpage = alloc_acid_page();
            dpage->page_id = page_id;
            dpage->jbuffered = false;
            dpage->new_page_cow = 0;
            dpage->plock = 0;
            if (rbtree_insert(&dpage->node, &ah->dpage_index) != 0)
                throw("dirty page index is corrupt", exception_fatal);
            ah->n_dpage_index++;
            // Allow page to be freely mutated via the MMU.
            strict_mprotect(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE);
        } else {
            // Page already dirty, this happens when two threads both try to
            // read a protected page before it has been marked dirty.
            // Try reading one byte from it while we have the acid handle lock.
            // This is a sanity check that should make the program crash
            // instead of becoming stuck in a page miss loop.
            *((volatile uint8_t*) page_addr);
        }
    } atomic_spinlock_unlock(&ah->ilock);
}

static void acid_open_fd(acid_h* ah, fstr_t path, int32_t* fd_ptr) { sub_heap {
    *fd_ptr = open(fstr_to_cstr(path), O_RDWR | O_CREAT | O_CLOEXEC, 0644);
    if (*fd_ptr == -1)
        RCD_SYSCALL_EXCEPTION(open, exception_io);
    // We aquire an exclusive lock on the files opened to avoid corruption.
    int32_t flock_r = flock(*fd_ptr, LOCK_EX | LOCK_NB);
    if (flock_r == -1)
        RCD_SYSCALL_EXCEPTION(flock, exception_io);
    // We force any files opened to be at least one memory page large.
    if (strict_fstat(*fd_ptr).st_size < PAGE_SIZE)
        strict_ftruncate(*fd_ptr, PAGE_SIZE);
}}

acid_h* acid_open(fstr_t data_path, fstr_t journal_path, void* base_addr, size_t new_length) {
    // Round new_length up to nearest page.
    new_length = ((new_length + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    // We're dealing with state that persists far longer than a fiber.
    // Ignoring region based memory allocation and using a global handle.
    acid_h* ah = vm_mmap_reserve(sizeof(acid_h), 0);
    *ah = (acid_h) {
        .fd_data = -1,
        .fd_journal = -1,
    };
    try {
        // Initialize page indexes.
        rbtree_init(&ah->dpage_index, cmp_acid_page);
        ah->n_dpage_index = 0;
        rbtree_init(&ah->npage_index, cmp_acid_page);
        ah->n_npage_index = 0;
        // Open data and journal.
        acid_open_fd(ah, data_path, &ah->fd_data);
        acid_open_fd(ah, journal_path, &ah->fd_journal);
        // Read current data length.
        ah->data_length = strict_fstat(ah->fd_data).st_size;
        // Map the journal and read it.
        size_t total_jsize = strict_fstat(ah->fd_journal).st_size;
        void* journ_map = strict_mmap(0, total_jsize, PROT_READ, MAP_PRIVATE, ah->fd_journal, 0);
        jrnl_idx_t* jidx = journ_map;
        ah->commit_rev = jidx->commit_rev;
        // Check if the journal contains uncommited changes.
        if (journ_verify(journ_map, total_jsize, ah->data_length)) {
            // Has uncommited changes from sudden shutdown. Apply them now.
            journ_commit(ah->fd_data, journ_map);
            // Wait for data file to sync.
            strict_fsync(ah->fd_data);
        }
        // Truncate the journal to throw away all pages and only preserve the journal header.
        strict_ftruncate(ah->fd_journal, PAGE_SIZE);
        // Unmap the journal.
        strict_munmap(journ_map, total_jsize);
        // Journal is commited and data is not mapped yet.
        // We can now safely execute a requested data resize.
        if (new_length != 0 && ah->data_length != new_length) {
            strict_ftruncate(ah->fd_data, new_length);
            strict_fsync(ah->fd_data);
            ah->data_length = new_length;
        }
        // Map memory.
        ah->base_addr = strict_mmap(base_addr, ah->data_length, PROT_READ, MAP_PRIVATE, ah->fd_data, 0);
        // Register sigsegv region handler.
        ah->srh = rsig_segv_rhandler_set(acid_rsig_segv, ah->base_addr, ah->data_length, ah);
        assert(ah->srh != 0);
        // Spawn thread that handles sync of journal.
        int32_t clone_flags = lwt_get_thread_clone_flags();
        ah->sync_stack = vm_mmap_reserve(VM_SYNC_THREAD_STACK_SIZE, 0);
        ah->sync_heap = vm_heap_create(0);
        void* stack_entry_point = (void*) vm_align_floor((uintptr_t) ah->sync_stack + VM_SYNC_THREAD_STACK_SIZE, VM_ALLOC_ALIGN);
        int32_t clone_r = lwt_start_new_thread(acid_sync_thread, stack_entry_point, clone_flags, ah, &ah->sync_physt);
        if (clone_r == -1)
            RCD_SYSCALL_EXCEPTION(clone, exception_io);
        ah->sync_pid = clone_r;
        return ah;
    } catch (exception_any, e) {
        acid_close(ah);
        throw_fwd("opening acid database failed", exception_io, e);
    }
}

void acid_expand(acid_h* ah, size_t new_length) {
    // Round new_length up to nearest page.
    new_length = ((new_length + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    // We are done if expanding to a length that is not greater.
    if (ah->data_length >= new_length)
        return;
    // Do expand. We lock index during critical operations.
    atomic_spinlock_lock(&ah->ilock); {
        // Resize the data file.
        strict_ftruncate(ah->fd_data, new_length);
        // Map more more memory. We can map anonymous memory since the new
        // data should be zeroed and we would make a private map anyway.
        int flags = (MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE);
        strict_mmap(ah->base_addr + ah->data_length, new_length - ah->data_length, PROT_READ, flags, -1, 0);
        // We need to wait on fsync to prevent invalid offsets in journal.
        strict_fsync(ah->fd_data);
        // Resize the custom signal handler.
        rsig_segv_rhandler_resize(ah->srh, new_length);
        // Note the new length.
        ah->data_length = new_length;
    } atomic_spinlock_unlock(&ah->ilock);
}

fstr_t acid_memory(acid_h* ah) {
    fstr_t acm = {.len = ah->data_length, .str = ah->base_addr};
    return acm;
}

fiber_main acid_wait_fiber(fiber_main_attr) { try {
    ifc_park();
} catch (exception_desync, e); }

void acid_fsync(acid_h* ah) {
    rcd_fid_t wait_fid;
    bool futex_wake = false;
    atomic_spinlock_lock(&ah->ctrl.lock); {
        switch (ah->ctrl.op) {{
        } case sync_op_none: {
            ah->ctrl.op = sync_op_commit;
            futex_wake = true;
            break;
        } case sync_op_commit: {
            break;
        } case sync_op_quit: {
            throw("undefined behavior: using while closing acid handle", exception_fatal);
        }}
        wait_fid = ah->ctrl.commit_wait_fid;
        if (wait_fid == 0) {
            fmitosis {
                wait_fid = spawn_static_fiber(acid_wait_fiber(""));
            }
            ah->ctrl.commit_wait_fid = wait_fid;
        }
    } atomic_spinlock_unlock(&ah->ctrl.lock);
    // Wake sync thread.
    if (futex_wake) {
        ah->ctrl.op_futex = 1;
        strict_futex(&ah->ctrl.op_futex, FUTEX_WAKE, INT32_MAX, 0, 0, 0);
    }
    // Wait for fsync.
    ifc_wait(wait_fid);
}

bool acid_snapshot(acid_h* ah) {
    rcd_fid_t wait_fid;
    bool futex_wake = false;
    atomic_spinlock_lock(&ah->ctrl.lock); {
        if (ah->ctrl.op == sync_op_quit)
            throw("undefined behavior: using while closing acid handle", exception_fatal);
        // We are not intrested in starting a commit if sync thread is busy fsyncing.
        if (ah->ctrl.in_sync_phase) {
            atomic_spinlock_unlock(&ah->ctrl.lock);
            return false;
        }
        // Order commit if not already ordered.
        if (ah->ctrl.op == sync_op_none) {
            ah->ctrl.op = sync_op_commit;
            futex_wake = true;
        }
        // Start wait for snapshot fiber.
        wait_fid = ah->ctrl.snapshot_wait_fid;
        if (wait_fid == 0) {
            fmitosis {
                wait_fid = spawn_static_fiber(acid_wait_fiber(""));
            }
            ah->ctrl.snapshot_wait_fid = wait_fid;
        }
    } atomic_spinlock_unlock(&ah->ctrl.lock);
    // Wake sync thread.
    if (futex_wake) {
        ah->ctrl.op_futex = 1;
        strict_futex(&ah->ctrl.op_futex, FUTEX_WAKE, INT32_MAX, 0, 0, 0);
    }
    // Wait for snapshot.
    ifc_wait(wait_fid);
    return true;
}
