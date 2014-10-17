I'd like to announce the open sourcing of librcd, a library that I have developed at my startup Jumpstarter since 2012. Librcd was initially a C library but it grew into a C dialect. It's design is inspired by Erlang, Go, D, Rust, CSP concurrency and functional programming in general.

[Github](https://github.com/jumpstarter-io/librcd), 
[install instructions](https://github.com/jumpstarter-io/librcd/wiki/Installing).

The primary goals of librcd are:

- Rapid Development. (Low cost.)
- Concurrency. (Not parallelism.)
- Correctness. (Compile and forget.)
- Cleaness. (Less boilerplate.)

I would say that it's most interesting features are:

- non-buffered lock-free concurrency with syntactic sugar
- lightweight fibers (green threads)
- region based memory allocation
- segmented stacks
- fixed (length coded) strings
- opaque non blocking io (userspace scheduling)
- exceptions
- regex code generator
- dict/lists template syntax

### How librcd is different from C

Librcd is designed to supersede libc based standard libraries and is a "language" and set of conventions to write high level network & server oriented applications. The build output is currently assumed to be a statically linked, self sufficient x86-64 binary that can be compiled and run on any modern x86-64 Linux system without dependency requirements. It's a C library at its core but it requires compile-time (frame lowering) code generation that [only my llvm branch support at the moment](https://github.com/jumpstarter-io/llvm). I made several attempts to implement everything without extending the language but unfortunately some things where just impossible to design with macros in a satisfactory way. I therefore added optional non standard syntax that is translated to C in a "post preprocessor" step that is run after the preprocessor.

When writing userspace Linux software in C it is generally assumed that you are using glibc. This assumption is so strong that the Linux manual, in many cases, does not even document the raw interface for syscalls but the glibc wrapper. Arguably the userspace in Linux IS glibc. It's such an implicit dependency that you need to pass special flags to gcc/clang to NOT have it dynamically linked with your program. I have gotten the impression that few programmers even think about the fact that "system headers" is actually glibc headers, which further equates the Linux "system" with glibc.

Librcd does not use glibc and considers libc deprecated. It's written to be run on Linux, not glibc. There's no system headers, dynamic linking or dependencies in librcd that make your program (and it's correctness) a function of other dependencies that happens to be installed in a system. Instead you are recommended to include all core symbols in librcd with a single initial `#include "rcd.h"`. Forget guessing what features goes in which headers and carfully listing the includes you need. Just include rcd.h and you should be good to go.

### Hello world in librcd

You can write vanilla C in librcd if you want but as soon as the post preprocessor encounters a: `#pragma librcd` line, magic starts, and the post preprocessor will translate literal strings to fixed length strings (`fstr_t`). Here's an example:
  
    #include "rcd.h"
    
    char* hello_c = "Hello world from libc!";
    
    #pragma librcd
    
    fstr_t hello_rcd = "Hello world from librcd!"

A fixed string has the following type as defined in `include/fstring.h`:

    typedef struct fstr {
        size_t len;
        uint8_t* str;
    } fstr_t;

To calculate the length of hello_c you would traditionally use `strlen(hello_c)` but with fixed length strings you can simply use the `len` attribute. A programmer which try to use `strlen` might be suprised that the symbol is not available by default as librcd assumes you're using fixed length strings for everything. Fixed strings is not guaranteed to be null terminated. This means that it's not safe to pass their `str` pointer directly to functions like `printf`, but fear not, librcd consistently uses fixed strings everywhere and null terminated strings nowhere. This allows you to do anything you could do in `libc`:

    printf(hello_c);
    
This can be written as:

    rio_t* h = rio_stdout();
    rio_write(h, hello_rcd);

There are sensible reasons why stdout has to be opened as a stream in librcd which is related to the networking and I/O design which we'll talk about later. You could also alternative write directly to stderr with `rio_debug` which uses separate synchronous I/O logic which is more suitable for debug and log printing:

    rio_debug(hello_rcd);

For more information, see the I/O library in `include/rio.h`.

The `main()` function has also been replaced with a function with the following signature:

    void rcd_main(list(fstr_t)* main_args, list(fstr_t)* main_env)

The green threads in libcd are called "fibers" and the main function is called from a "main fiber". There's nothing special about the main fiber compared to other fibers. If a fiber returns from it's root function it's simply discarded. To terminate the process with a return code we instead call a function called `lwt_exit`.

We use the above information to produce a complete hello world:

    #include "rcd.h"
    
    #pragma librcd
    
    void rcd_main(list(fstr_t)* main_args, list(fstr_t)* main_env) {
    	rio_t* h = rio_stdout();
    	rio_write(h, "Hello world!");
    	lwt_exit(0);    
    }

The last big difference from C is that librcd also comes with another pragma that generates code for regular expressions and there's syntactic sugar for concurrency but we'll talk more about that later. Let's first discuss how we should run this hello world.

### Building and project conventions

Librcd does not use make/makefiles. I believe that a turing complete language to write build instructions in is broken by design. The project should not be concerned with the exact steps required to build it as this will unavoidable require assumptions that may not be true. For example, if you have a list of sequential compile instructions one of the incorrect assumptions made is that compiling is always a sequential and not sometimes a parallel process. Ideally the project should only describe itself in the most minimal way possible and leave the rest to the build tools. A useful definition of a "minimal way" is the minimal way which is still readable without forcing the build tools to scan source code and make guesses based on heuristics.

Instead we created a small python hack called occ that introduces conventions for how a C project should be built and structured. All projects has a minimal JSON file called `occ.project` which lists all information required to make the project. All projects in a users system is indexed by occ so the dependency graph can be resolved automatically. It also solves problems like parallel build and caching. It will only recompile the minimum required source code when a file has changed.

[https://github.com/jumpstarter-io/occ](https://github.com/jumpstarter-io/occ)

The file and directory convention occ uses is currently the following:

- `/build/` - Directory for build output.
- `/include/` - Where libraries place their public headers.
- `/libs/` - Extra dependencies that needs to be compiled with custom tools. All .build files are executed and should return a list of object files for the linker.
- `/template/` - Template for new projects created with occ `-i` mode.
- `/tools/` - Additional binaries and scripts that could be used in the build toolchain. It will be added to `PATH` by default.
- `/src/` - Source code and internal headers. All .c files are compiled here by default.
- `occ.project` - Project file in JSON.

I'm not fully satisfied with the conventions and implementation of occ right now, but it works. There's several problems that make it hard to work with specific versions of dependencies and multiple copies of the same project in a system. These are problems that tools like npm solves but occ doesn't since we never had them. We have always worked with the latest version of all dependencies until now. I would happily accept help and ideas to fix those problems, including radical solutions like building a similar tool from scratch in another language, or making npm capable of replacing occ.

Let's go back to our hello example, if you followed the install instructions you should be able to run occ from the command line. First go to wherever you placed the librcd source code and index it so occ can find it. You have to do this once:

    cd ~/projects/librcd
    occ -x

Now, to create a simple "hello" project, go to whatever folder you want to want your projects to be in and initialize the project:

    cd ~/projects
    occ -i librcd hello
    cd hello

We want to replace the standard main file with a new one:

    rm src/*
    cat > src/hello.c
    # paste the above hello world example here and press ctrl+d

The default `occ.project` file should not need to be changed. Occ uses the name of the project folder as the name of the project. We make sure we're standing in the project root and compile it with the `debug` mask:

    occ -c -m debug
    
This should produce a binary in the build folder that you can run. You can change the name of the binary by changing the value of the `output` key in `occ.project`.

### Region based memory allocation

Manual memory managment is a big productivity and code readability killer in C. Memory allocation also make library interfaces clumsy since you have to worry about the possibility that any allocated memory needs to be managed by the callee and can not just be leaked and thrown away when the callee don't need it anymore. Worst of all, manual memory management tend to introduce leaks since it's usually hard to read code and see that it's leak free without reading and understanding the context. This is what I'd like to call "expensive correctness". Garbage collected languages is a thing because programming and economics are two sides of the same coin.

C programmers generally prefer allocating memory on the stack since it gives the memory a lifetime that is managed by the compiler instead. The desire to avoid the heap is so strong that programmers sometimes resort to anti patterns like max allocations with magic constants which is a classic opening for exploits. But in many cases stack allocation is simply not applicable. The stack can't be used with dynamic allocations in loops or for memory that needs to be passed out of the frame.

It could also be useful to have allocations with a lifetime that's only valid in a small section of the frame. For example some temporary memory required to serialize some values and concatenate a debug print. As soon as the print is complete that memory can be thrown away. Some implementations of `printf()` avoids concatenation by (inefficiently) printing every token with its own system call.

Manual memory managment also gets really complicated when you start to mix it with concurrency in C. Memory could be owned by one, none or many different actors making the correctness exponentially harder to understand.

Librcd has attempted to solve this problem with region based memory allocation. Essentially every fiber in a librcd program is allocating memory on it's own separate "region" or heap. Instead of calling `malloc()` and `free()` which allocates memory globally librcd programs use `lwt_alloc_new` and `lwt_alloc_free` which does local region allocations (although these are rarely called directly). Sub heaps are created as a block statement and when leaving the sub heap (in any way, including through goto) the memory is automatically free'd:

    sub_heap {
        fstr_mem_t* line = readALineFromSomewhere();
        sub_heap {
            fstr_mem_t* out = conc("You entered: [", line, "]");
            rio_debug(fss(out));
        }
        // "out" memory is has been free'd here
    }
    // "line" memory is free'd here, no leaks
    
The above example uses `fstr_mem_t*` which is a handle to the actual fixed string allocation, very useful if you actually want to manage the memory manually. Functions that allocate a string usually return a `fstr_mem_t*` telling the callee "here, you own this now" while functions generally take a `fstr_t` since they don't care who owns the memory. Casting a `fstr_mem_t*` to a `fstr_t` is done with `fstr_str()`. This is done so often that it has a shorthand `fss()` macro reseved for it. The reverse cast is not possible.

Memory allocated in sub heaps can be escaped to allow it to survive when the region is exited. Here's an example of the actual implementation of `fstr_replace`:

    fstr_mem_t* fstr_replace(fstr_t source, fstr_t find, fstr_t replace) { sub_heap {
        return escape(fstr_implode(fstr_explode(source, find), replace));
    }}

In the above example `fstr_explode` leaks a linked list of strings into the fstr_replace frame. This list is then glued together with `fstr_implode` in a newly allocated `fstr_mem_t*`. This allocation is then escaped to the callee heap while returning. The root sub heap of the function prevents all memory leaks.

Also note that in librcd all dynamic allocations should generally be done with the heap and not the stack. Using VLAs and alloca() is supported but pointless since they are implemented as calls to the memory allocator and not with the stack. This is because the stack itself is allocated on the heap since librcd uses segmented stacks.

In general sub_heaps and escaping gets you quite far but when state truly needs to be passed through an cyclic runtime graph that can't be expressed as a tree you start to require more advanced tools for managing memory. One such example is implementations of data structures. Therfore it's also possible to create "context less" heaps with `lwt_new_heap()` (which are free'd and connected to the current heap just like any other allocations), switch to that heap with `switch_heap` and then import allocations with `import`. 

When passing "complex" data structures around which is not just flat allocations (for example structs with pointers) that needs to be passed around through various contexts, one useful pattern is to allocate a heap for it and connect it to the root struct. This is how exceptions are implemented in librcd, see the source code and documentation for more information on these advanced patterns.

Overall region based allocation is not a silver bullet but in most practical cases it's good enough. The most important property is that it usually allows you to catch any leaked memory and throw it away without being concerned about pointers and free calls. One classic pitfall I've obvserved in programmers that are new to region based memory allocation is micro managing all the memory religiously and making the code an unreadable mess of sub heap blocks and escapes. Don't be affraid to leak memory from functions when it's not strictly neccesary to clean it up immediately. It's a form of premature optimization that usually make the performance worse.

### Fixed (length encoded) strings

C heavily encourages pointer arithmetic oriented design, including parsing and string handling. Librcd considers pointer arithmetics unsafe (i.e. expensively correct) and should be avoided. Instead librcd comes with a "fixed string library" in `fstring.h` that provides many general alternatives that works for any data. A null byte is just another byte. There's no specific abstractions for strings, text or binary, `fstring.h` can be used for everthing.

Say we have a string and want to trim away all whitespace around it and replace all tokens of "carrots" with "bacon".

    static fstr_mem_t* baconify(fstr_t raw_str) {
        return fstr_replace(fstr_trim(raw_str), "carrots", "bacon");
    }
    
That's it! Note that we're returning a `fstr_mem_t*` and not a `fstr_t` because we're passing a string allocation back to the caller. "You own this new string allocation now and you probably want to free it when you're done." Here's using it:
    
    fstr_t str = fss(baconify("   I love carrots!!! Yum, carrots.\n  "));
    assert(fstr_equal(str, "I love bacon!!! Yum, bacon."));
    
The `fss()` converts a `fstr_mem_t*` to a `fstr_t`.
    
Let's do something harder: parsing a textfile where each line has "key: value". We want to write the value of a certain key to a stream. The line endings could be either \r\n or \n. There can also be arbitrary amounts of surrounding whitespace and even empty lines that should never be written. Also, allocating new memory is cheating, let's avoid the heap completely this time when parsing:

    static bool print_line_key(rio_t* stream, fstr_t raw_input, fstr_t key) {
        // Loop line by line.
        for (fstr_t line; fstr_iterate_trim(&raw_input, "\n", &line);) {
            // Divide key from value.
            fstr_t line_key, line_value;
            if (!fstr_divide_trim(line, ":", &line_key, &line_value))
                continue;
            // Print value if key matches.
            if (fstr_equal_case(key, line_key)) {         
                rio_write(stream, line_value);
                return true;
            }
        }
        return false;
    }

Note that `fstr_iterate()`, `fstr_divide()` and friends is impossible to implement with null terminated strings without modifying the original string (which is controversial) in a generic string library. Now let's use the function from main:
    
    void rcd_main(list(fstr_t)* main_args, list(fstr_t)* main_env) {
        // Parse arguments.
        fstr_t path, key;
        list_unpack(main_args, fstr_t, &path, &key);
        // Read file and parse it, scanning for the specified key.
        fstr_t raw_input = fss(rio_read_file_contents(path));
    	rio_t* stream = rio_stdout();
    	bool found = print_line_key(stream, raw_input, key);
    	lwt_exit(found? 0: 1);
    }

Compiling and using the program:
    
    > echo -e " coffee: black  \n tomato:red\n\t cucumber :\tgreen\n lemon   :yellow\nsalt:white" > colorsofthings
    > ./build/getlineval colorsofthings cucumber
    green
    > ./build/getlineval colorsofthings lime
    > ./build/getlineval colorsofthings coffee
    black

Even simple low level pointer arithmetics can be replaced with safer fixed string based logic. Let's take this function which replaces all non ascii values in a null terminated string with "`[?]`" and prints the final string:

    void ascii_print_str(char* x) {
        // Worst case is that each in byte will be replaced with 3 bytes.
        char o_buf[strlen(x) * 3];
        char* o = o_buf;
        for (;*x != 0; x++) {
            if (*x < 0x80) {
                *o++ = *x;
            } else {
                *o++ = '[';
                *o++ = '?';
                *o++ = ']';
            }
        }
        *o++ = '0';
        printf("%s", o_buf);
    }

The above C function has undefined behavior due to a subtle off-by-one error. The final null terminator is written beyond the end of the buffer in the worst case. This should corrupt the stack or heap in the rare case when `o` happens to have a size dividable with 8 or 16 as the VLA allocation won't get trailing alignment padding. It's a pretty sevre form of undefined behavior since the severity of a bug increases with how rarely it reproduces.

Let's compare it with the following fstr based function:

    void ascii_print_str(fstr_t x) { sub_heap {
        // Worst case is that each in byte will be replaced with 3 bytes.
        fstr_t o_buf = fss(fstr_alloc(x.len * 3));
        fstr_t o_tail = o_buf;
        for (size_t i = 0; i < x.len; i++) {
            if (x.str[i] < 0x80) {
                fstr_putc(&o_tail, x.str[i]);
            } else {
                fstr_putc(&o_tail, '[');
                fstr_putc(&o_tail, '?');
                fstr_putc(&o_tail, ']');
            }
        }
        rio_debug(fstr_detail(o_buf, o_tail));        
    }}

Instead of pretending that the variable allocation belongs on the stack when it's dynamic we go with a proper heap allocated buffer and a dynamic length. The sub heap throws away the buffer when the function returns. The function then uses a recurring fixed string theme when working with buffers: tail buffers. The tail buffer is the tracked end range of the buffer that has not yet been written.

The call to `fstr_putc` inserts a character at the start of the tail buffer and shrinks it from the front. If the tail buffer has no remaining space the `fstr_putc` call will return false, a return value we choose to ignore since we assume the buffer is large enough.

Finally the function inverts the tail of the buffer by calling `fstr_detail` which returns the buffer head that was written. The head is then printed with `rio_debug`. If we made an incorrect buffer size estimation we simply have an incorrect (but well defined) truncated result returned by the function instead of a potential security hazard and segmentation failure.

The lost speed of calling a function and checking the length is neglible in most applications because `fstr_putc` is always inlined. If you really care about optimizations though you shoud rather worry about memory access and CPU caching than counting instructions. A modern CPU does not evaluate code by excuting one instruction at a time.

While we're talking about buffers it's also worth mentioning that the real world is full of cases where you have to max allocate a buffer and there is no possible buffer size estimation, you just have to pick a sensible limit. A concrete example is reading a HTTP header. In this case it's worth knowing that you can allocate strings with `fstr_alloc_buffer` instead of `fstr_alloc`. The former will give a minimum length and not an exact length, so the allocation is actually rounded up in the memory allocator when the length is aligned. This prevents wasting extra bytes that could have gone to a more generous limit.

### Throwing and catching exceptions

A recurring problem programmers face is writing a series of instructions which could fail while similtaniously having side effects on the memory and kernel that should be gracefully reverted in case of failure and handled by a more generic failure handler. This is essentially the "transaction" pattern where you do everything in an easy to read transaction that handles the common and most expected (non exceptional) case and in case of failure unwinds all side effects the transaction had. In standard C the classic way to implement this is to jump to a cave where you carefully look at the state and unwind it in case of failure. This is boiler plate that bogs down the codebase and is also expensively correct since every jump need to be individually triggered and tested. A better solution is to unwind the heap, the stack and all other side effects made up until that point while vertically passing an error value back to a more generic context. This is what exceptions do.

Failure can sometimes be so unexpected that catching makes no sense. The program *should* crash. If a fiber in librcd does not catch an exception the whole program crashes. Some programs need a higher degree of robusness. In this case it can be more desirable to catch the exception, print relevant debug information and then just cancel that part of the program. This is very useful when programming server components.

Exceptions is also very useful in concurrency as unsyncronized execution contexts can have a large combination of meaningless states relative each other which should be handled by cancellation, retrying or other generic logic.

Librcd implements support for exceptions through the use of macros, `setjmp`/`longjmp` and some gcc C extensions that are implemented by clang. When throwing exceptions all regions entered after entering the try region will be unwinded. Throwing an exception in a sub heap will therefore dealloate all memory allocated in that region.

As an example we'll set up a server that reads some numbers from stdin and adds them together. The numbers are decimal and separated by line endings. The server should be strict and throw an exception if the format is not recognized.

    
    
[Read more about exceptions in the documentation](https://github.com/jumpstarter-io/librcd/wiki/Exceptions).

### Opaque non blocking I/O & multi threading

Nomally when writing networked applications in C a programmer has to resort to one of two evils: Either using resource demanding pthreads (and forgetting about 10CK) or polluting the source code by structuring all I/O around a central select/epoll call. Librcd brings the best of both worlds through an userspace scheduler. The scheduler uses edge level triggered I/O interrupts from it's own epoll call loop and concurrency interrupts to enqueue fibers to be run. The jump in userspace saves an expensive kernel context switch and prevents fibers from starving each other from preemption overhead under heavy load. This is commonly known as "cooperative scheduling". What it means is essentially that you can write blocking code that is easier to read and let librcd take care of the context switching without bothering the kernel.

When librcd starts it spawn exactly as many worker threads as it detects cores in the system. This provides opaque multi threading without any special syntax. If two fibers can be run in parallel (because they are not "joined") the scheduler probably will if the system has more than one core. But it's also up to the Linux kernel: librcd worker threads have currently no core affinity and if Linux forces the program to run on one core all workers will. Correct code should not depend on or care about the time domain(s) it's executed in.

Currently the librcd scheduler is very naive compared to the scheduler in golang. It has a global run queue of fibers which causes contention and wasted time in spinlocks. Fibers also have no "inertia" that reduces the rate of thread/core switches. This casues unneccesary CPU cache misses which is especially harmful in NUMA configurations. But what it lacks in performance it makes up for in simplicity. It should be easy to improve if needed and also simple to debug.

Here's an example:

    fstr_mem_t* read_line() { sub_heap {
        fstr_t buf = fss(fstr_alloc_buf(0x1000));
        fstr_t line = rio_read_to_separator(rio_h, buf, "\n");
        return escape(fstr_cpy(line));
    }}
    
The call to `rio_read_to_separator` does I/O and possibly defers in case the read request blocks. This creates a cancellation point, a concept you may be familliar with from ptheads. In pthreads a cancellation results in a specific return code, while in librcd a cancellation exception is thrown instead. It also means it's a point where the thread could context switch and run another fiber in case the read blocks. This means we can't be sure we're still executing on the same thread after the function returns. This is not a problem since it's undefined on what thread code is running on in librcd and code should not be written to make such assumptions.

### Concurrency is not parallelism

Librcd is not primarily designed for paralleism although it's capable of that. [It's written for concurrency, a completely different, and much harder problem](http://blog.golang.org/concurrency-is-not-parallelism). Specifically it's written to support actor based concurrency.

When I began programming in C I used pthreads with locks but soon found myself in "lock hell" when writing more advanced software. The combination of locks and interacting dynamic contexts is something that can easily kill a project when it runs into an unsurmountable cliff of complexity. I had already looked into Erlang and finally escaped lock hell by utilizing lock free actor based concurrency instead. I found that it often forced me to reason about my problems before coding in a good way that lock based programming never did. It allowed me to start writing code where the correctness was something you could reason with. Lock based programming rather lulls you into a false sense of security where code appears to work while having dangerous races or deadlocks that can strike at the worst moment and be hard to impossible to debug. I have never spent more than a few hours to understand a concurrency issue in librcd while I could easily throw away several days on just reading code to figure out what was wrong with my pthread program.

Actor based concurrency is based on a few core ideas that you may find controversial if you are used to pthreads. Avoid sharing state whenever possible. This means that globals, locks, "conditions" and static memory are no longer primitives you design your software with. Instead of thinking about concurrency like a way to protect memory, instead imagine that this memory (or state) belongs to actors and concurrency is describing the way these actors interact with each other. State can only be shared through these interactions. Erlang uses "message passing" for this while golang uses "channels". When I designed librcd however I found these primitives too "high level" and required too much memory managment magic and syntatic sugar to be suitable for C. Instead I opted for a syntax where functions could be "concurrent" instead and essentially be called from two contexts at once, effectively joining the execution of two fibers into a single execution stream. 

I will now use pseudo code to describe the librcd concurrency principles in a more detailed way. Librcd has "normal" functions `f(x) -> z` where we call `f` with the state of type `x` which should return state of the type `y`. Librcd also has "concurrent functions" which can be called from two contexts: `f(x | y) -> (z | w)` where `x` is the client argument type, `y` is the server argument type, `z` is the return type for the client and `w` the return type for the server. These can be called either as a client with `f(x, i) -> z` where `x` is the client state and `i` is the identity of the fiber to join. They can also be called as a server with `f(y) -> w`. In the server case we call this "accepting" rather than "calling" but it's in priciple the same thing once the function is running. Before the call there is however some significant differences. First of all a client calls a specific server while the server will accept any client. The server can also accept multiple functions at once. For example: `{f,g,h}(y) -> w` will accept any call to the functions `f`, `g` or `h` which all takes `y` and returns `w`.

I like this concurrency primitive because it allows modeling of problems with functions instead of having locks or any special loose state that travels between actors. It abstracts how a function call was synchronized so we can ignore it. Either the client was blocked, or the server or none or both. It doesn't matter and will not pollute the concurrency logic. We can also see that this is a more generic concurrency primitive than locks since a lock can be described as a special case in this notation where the server continously accept synchronization of a state like: `g(y): g(f(y))`.

An actor needds an identity. When Keanu Reeves is acting in the Matrix his identity is "Neo" and when nginx is acting web server it's identity can be "4359". The actor identity is the contact point which other actors use to reference and interact with them and is thererfore very important. In memory managed languages this reference can be some form of "object" and is therfore always unique. It is also possible to allow references which when no longer used and collected, destroys the actor it points to, which can be quite useful. C is however not memory managed and the usual solution is to either use a pointer (e.g. `pthread_t*`) or by using a numeric descriptor (e.g. UNIX process ids or file descriptors). A big practical problem with pointer references is that the actor cannot be asynchronously killed and cleaned up while references to it still exist and when used, causes invalid memory access. This is a common pitfall for developers and it makes dynamic concurrency hard to implement and expensively correct. It forces the developer to carefully orcestrate cleanup and severely limits the design possibilities of concurrent architectures. You cannot just pass the dynamic reference to a foreign context, like a generic library, without involving it in the cleanup somehow, and that logic is itself usually prone to deadlock and requires large amounts of boilerplate.

In my opinion pointer based references is just an unneccesary restriction and the performance benefit is usually not worth it. It's better to have a virtual, numeric ID instead of a memory reference as this has significant economic benifits caused by simpler software design. But even this solution can be compromised, and usually is. If IDs are allowed to be recycled you're to be back on square one. When a ID can be reused you still have the problem where the cleanup has to be synchronized and orchestrated as the ID could refer to a new, different actor after the actor is killed. We want IDs that are unique forever so we don't have to worry about the time domain they are used in. In my opinion it was a great historic mistake to allow process ids and file descriptors to be recycled in UNIX and this has caused countless bugs. A classic mistake in UNIX is to access a file descriptor at a point that is not synchronized with a close operation, possible causing I/O to a different file descriptor. A classic bad fix to that problem is checking `errno` for `EBADF`. Checking for `EBADF` is broken by design and objectivelly useless since it will only work if you win the race condition where the file descriptor have not happened to been reused yet.

The fiber id in librcd (`rcd_fid_t`) is instead a 128 bit number (`uint128_t`). This is large enough that the possiblity of wrap around can be completely dismissed and the life of a fiber is represented with a handle that is assumed to be unique forever. Librcd also has a `rcd_sub_fiber_t*` and it's relationship with `rcd_fid_t` is analogous with the relationship between `fstr_mem_t*` and `fstr_t`. The sub fiber pointer represents ownership of a fiber which will be synchronously cancelled and cleaned up on exit when the allocation is free'd. The `rcd_fid_t` is however a global reference that can be used anywhere and shared wildly.

Let's look at some actual examples. Assume we have a program which receives warnings from a sensor and as soon as we receive one we should do a remote internet API call to a system that logs how many times the warning was triggered. This remote API could be down some times in which case the program should retry the API call until it's successful. Because this API call can take unspecified time we want to concurrently keep counting the triggered warnings and report the number of warnings since the last succesful report. Librcd comes with a small library called ifc which has solutions for common concurrency problems. It includes an "event" solution we could use here that is analogous to [eventfd(2)](http://man7.org/linux/man-pages/man2/eventfd.2.html). It's implemented as follows:

    join_locked(uint64_t) event_fiber_trigger(uint64_t trigger_count, join_server_params, uint64_t* current_count) {
        uint64_t current_count_left = (ULONG_MAX - *current_count);
        uint64_t add_count = MIN(trigger_count, current_count_left);
        *current_count += add_count;
        return trigger_count - add_count;
    }

    join_locked(uint64_t) event_fiber_consume(join_server_params, uint64_t* current_count) {
        uint64_t return_count = *current_count;
        *current_count = 0;
        return return_count;
    }

    fiber_main event_fiber(fiber_main_attr) { try {
        uint64_t count = 0;
        for (;;) {
            accept_join(
                event_fiber_trigger if (count < ULONG_MAX),
                event_fiber_consume if (count > 0),
                join_server_params,
                &count
            );
        }
    } catch (exception_desync, e); }

    rcd_fid_t ifc_create_event_fiber() {
        rcd_fid_t event_fid;
        fmitosis {
            event_fid = spawn_static_fiber(event_fiber(""));
        }
        return event_fid;
    }

    void ifc_event_trigger(rcd_fid_t event_fid, uint64_t count) {
        while (count > 0)
            count = event_fiber_trigger(count, event_fid);
    }

    uint64_t ifc_event_wait(rcd_fid_t event_fid) {
        return event_fiber_consume(event_fid);
    }

You can compare this implementation with [the kernel C implementation](https://github.com/torvalds/linux/blob/master/fs/eventfd.c) as they have similar functionality.

The program we mentioned can essentially trigger the event counter with a certain number of events. This will unblock the call to `ifc_event_wait` which will return the total number of events counted. The events are reported to the remote API and the `ifc_event_wait` is called again.

This example introduce the most fundamental concurrency syntax. First of all new fibers is spawned with the use of a `fmitosis` block. Biological cells go through a mitosis state when they duplicate and librcd fibers does as well. The block serves the purpose where a new heap can be created for the new fiber and state can be copied in, transfered or created before the fork. The reserved fiber id is available for immediate use as well through `new_fid`. As soon as `spawn_fiber` or `spawn_static_fiber` is called the fork is complete and the new fiber is now running. Note how the spawn wraps something that looks and is type checked like a normal function call. This is a huge relief from the `void*` casts required by `pthread_create` that throws type checking out of the window. The first argument to the new fiber is always it's name which is useful when debugging and can be set to an empty string when in doubt. This parameter is declared in the `fiber_main_attr` macro. The fiber id is returned from the function and can be used to call `ifc_event_trigger` or `ifc_event_wait`.

The `event_fiber_trigger` and `event_fiber_consume` are examples of the "concurrent functions" previously described. They are called from the client and from the server and effectively joins their execution stream. The identity of the executing fiber is however the client to avoid corner cases. A backtrace would list the client, not the server side, call stack. The `rcd_self` would also return the client fiber id. To get the server fiber id you would use the `server_fiber_id` declared by the `join_server_params` macro. This macro separates the client side arguments (left side) from the server side arguments (right side). Since C only allows returning one type from functions and not several, we reserve the return value to be used for the client. A return value for the server can be implemented through the use of pass-by-reference as we do in this case.

Calls to `event_fiber_trigger` and `event_fiber_consume` will block indefinitely until one of these conditions occour:

1. The specified server fiber chooses to accept the client fiber on this function.
2. Cancellation point: The client fiber is cancelled. The join aborted with an `exception_canceled`.
3. Cancellation point: The server fiber was or became dead. The join aborted with an `exception_join_race`.
4. Cancellation point: A server fiber the client is already joined with is cancelled. The join aborted with an `exception_join_race`.
5. The client is non interruptable and the server is killed or never existed. The join is aborted with an `exception_no_such_fiber`.

Don't think too much about join_races or try to be defensive and catch everything. The difference between `exception_no_such_fiber` and `exception_join_race` is also meaningless in 99% of all cases and can be ignored and categorized as "join failures". In general you should just let things crash as join_races are usually means that another fiber that is a prequisite for our existance has been killed and we should die (or unwind) as well. This can be done with a root `exception_desync` which catches both cancellations and join failures, which we use in `event_fiber`. We don't want to 
leak an exception from a `fiber_main` in a library as this will make it fatal and crash the entire program.

Let's say we want an implementation that does not leak a join race and throws an io exception instead. This is a common requirement for libraries that want to hide their internal concurrency. We would then rewrite `ifc_event_trigger` and `ifc_event_wait` like the following:

    void ifc_event_trigger(rcd_fid_t event_fid, uint64_t count) {
        try {
            while (count > 0)
                count = event_fiber_trigger(count, event_fid);
        } catch (exception_inner_join_fail, e) {
            throw_fwd("the event_fid was no longer valid", exception_io, e);
        }
    }

    uint64_t ifc_event_wait(rcd_fid_t event_fid) {
        try {
            return event_fiber_consume(event_fid);
        } catch (exception_inner_join_fail, e) {
            throw_fwd("the event_fid was no longer valid", exception_io, e);
        }
    }

It's important to catch join races with `exception_inner_join_fail`. Specifically catching `exception_join_race` is usually an error since we want to catch all join failures, no matter if the fiber is currently in an uninterruptible state or not. It would also be invalid to catch "outside" join races thrown in point 4 above caused by joins further down in the call stack. We forward the caught exception in the 3rd argument to `throw_fwd` to maximize context when debugging.

The server is accepting calls with `accept_join` which allows specifying one or more functions to accept on and the server arguments to be specified after the `join_server_params` token. All functions in the list must have compatible server argument types that is type checked at compile time. The `accept_join` also supports an extended `if` syntax which essentially allows specifying an expression which must be true to accept on that function. This allows us to block calls to `event_fiber_consume` and thus `ifc_event_wait` as long as the count is zero. We also use is to avoid integer overflow by blocking calls to `event_fiber_trigger` if we have no more room to count additional events.

Controlling what function calls should be blocked or not is a cornerstone in librcd concurrency design. It's however less powerful than the "guards" used in Erlang. There is no syntactic sugar that allows us to block if the number of events cannot be inserted all at once (atomically) so we need a `while (count > 0)` loop in `ifc_event_trigger`. We ignored this problem since this requirement is quite uncommon. Let's say we really want to atomically insert the number of events though, what would we do? The general solution is to use a dummy fibers lifetime to represent a more advanced/abstract condition that can be a function of both the client and server state. As an advanced excercise, here's an implementation that supports atomic event triggering:

    fiber_main ifc_pending_consume_fiber(fiber_main_attr) { try {
        ifc_park();
    } catch (exception_desync, e); }

    join_locked(rcd_fid_t) event_fiber_trigger(uint64_t trigger_count, join_server_params, uint64_t* current_count, rcd_sub_fiber_t** consume_ev_sf) {
        uint64_t current_count_left = (ULONG_MAX - *current_count);
        if (trigger_count > current_count_left) {
            if (*consume_ev_sf == 0) {
                server_heap_flip fmitosis {
                    *consume_ev_sf = spawn_fiber(ifc_pending_consume_fiber(""));
                }
            }
            return sfid(*consume_ev_sf);
        }
        *current_count += trigger_count;
        return 0;
    }

    join_locked(uint64_t) event_fiber_consume(join_server_params, uint64_t* current_count, rcd_sub_fiber_t** consume_ev_sf) {
        if (*consume_ev_sf != 0) {
            server_heap_flip {
                lwt_alloc_free(*consume_ev_sf);
                *consume_ev_sf = 0;
            }
        }
        uint64_t return_count = *current_count;
        *current_count = 0;
        return return_count;
    }

    fiber_main event_fiber(fiber_main_attr) { try {
        uint64_t count = 0;
        rcd_sub_fiber_t* consume_ev_sf = 0;
        for (;;) {
            accept_join(
                event_fiber_trigger if (count < ULONG_MAX),
                event_fiber_consume if (count > 0),
                join_server_params,
                &count,
                &consume_ev_sf
            );
        }
    } catch (exception_desync, e); }

    rcd_fid_t ifc_create_event_fiber() {
        rcd_fid_t event_fid;
        fmitosis {
            event_fid = spawn_static_fiber(event_fiber(""));
        }
        return event_fid;
    }

    void ifc_event_trigger(rcd_fid_t event_fid, uint64_t count) {
        for (;;) {
            rcd_fid_t consume_ev_fid = event_fiber_trigger(count, event_fid);
            if (consume_ev_fid == 0)
                return;
            ifc_wait(consume_ev_fid);
        }
    }

    uint64_t ifc_event_wait(rcd_fid_t event_fid) {
        return event_fiber_consume(event_fid);
    }

 
The `ifc_park()` ensure the execution deadlocks and can only return by throwing an exception. This gives us a fiber (`ifc_pending_consume_fiber`) that we can kill on demand and which uses as little resources as possible. It is allocated as a sub fiber. This allows us to automatically cancel and clean it up if the event fiber itself is cancelled as the `consume_ev_sf` will be free'd when cleaning up and freeing the root heap in `event_fiber`.

In the corner case where more events than possible is attempted to be triggered in `event_fiber_trigger` we flip to the server heap (the heap context is the client side by default). We then spawn a `ifc_pending_consume_fiber` and allocate a sub fiber handle to it in the server state. Finally the sub fiber id is returned with `sfid` which converts the `rcd_sub_fiber_t*` to a `rcd_fid_t`. It would be unsafe and undefined behavior to use the `rcd_sub_fiber_t*` outside the join as it's owned by the server, but it's completely safe to return a `rcd_fid_t`. We then use `ifc_wait` in `ifc_event_trigger` which waits for the fiber to exit. The fiber is killed in `event_fiber_consume` which allows us to wait for a consume before rechecking again if we have room to insert the events. Note that we need to switch to the server heap when freeing the handle as well or we would get a fatal error for trying to free memory in a heap context we're not standing in. Freeing memory manually is done with `lwt_alloc_free` which will invoke any related destructors for the memory.

If you are used to `pthreads` it can seem obscene to create a "thread" just for the purpose of conveying an event, but fibers are extremely lightweight and should not consume more than a couple of hundred bytes. They are built to synchronize very small states with the power of segmented stacks.

Some extra advanced concurrency notes: It's also possible to accept with `auto_accept_join` which is a more efficient way to unconditionally accept in a loop as it never returns. Fibers can be typed, this allows compile time type checking which is useful to prevent mixup of fiber ids. There is a `join_shared` alternative which allows multiple clients to accept in parallel which is useful when the clients only reads and does not change the server state. Finally it is possible to accept on the server side instead of the client side when joined by using `server_accept_join`. This pattern is useful when creating load balancers and pools where many clients join with many workers. More on this in another blog post.

### Segmented stacks

Segmented stacks also proved to be impossible to implement without compiler assistance. I'm a big fan of llvm/clang and fortunately the developers had already begun writing experimental support for [split stacks](http://llvm.org/releases/3.0/docs/SegmentedStacks.html), a feature that had already been [implemented in gcc](https://gcc.gnu.org/wiki/SplitStacks) in their aspiration to be as compatible with gcc as possible. I added [additional patches that introduced a librcd mode](https://github.com/jumpstarter-io/llvm/) (`--librcd-stack-segmentation`) with disabled red zone and prologues that calculates the exact amount of stack each frame require, as well as adapting the thread local memory access to be adapted to the librcd fiber memory layout. This made librcd compiler specific so *it will only compile with my custom branch of llvm 3.3 at the moment*. This also makes dynamic linking nearly impossible to support as code must be compiled with segmented stack support.

Essentially each function frame has a prolouge and epilouge injected which checks the required stack for the frame against a thread local limit. If an overflow is detected it context switches to the librcd system which allocates a new stack segment, copies over any call arguments passed on the stack and calls back. Code written in assembly (e.g. memcpy) has wrappers injected for their global symbols which switches to the thread "mega stack" if called from a librcd segmented stack context. This restricts assembly code from calling back to "high level" C code. Assembly code may only call other assembly code.

The segmented stack solution comes with performance characteristics that C programmers might not be used too. There are many controversies and drama surrounding segmented stacks that I will not talk about in this blog post because it has already been extensevly discussed already. For example, [here is rusts decision to abandon it](https://mail.mozilla.org/pipermail/rust-dev/2013-November/006314.html).

My personal position is that segmented stacks is neccesary to achive the low overhead of fibers and unlocks straight forward concurrency design where state is connected with actors/fibers in the largest possible extent. I personally explored the MMU solution first and was not happy with the performance and fiber overhead. Even 4K is simply way too much memory for a small fiber and you will quickly reach operating system limits like the linux overcommit protection (`/proc/sys/vm/overcommit_memory`) and the 64K max map count (`/proc/sys/vm/max_map_count`). It probably also has poor CPU cache characteristics.

I have never personally required optimizations that reduce the overhead from stacklet allocation but if I ever had to I know of several possible simple optimizations. A special function attribute could be added to clang which dramatically increases the stack requested (but not allocated) for a frame. This way a programmer could analyze their librcd program and find "hotspots" where time is wasted on allocating stacklets during pure CPU work and mark entry functions manually to increase stacklet size. This could also theoretically be done automatically. By analyzing the call graph in compile time you could identify leaf function groups that never context switch and perform pure CPU work.

Another solution is to do "stack copying". [This has been discussed in the golang community](https://groups.google.com/forum/#!topic/golang-dev/i7vORoJ3XIw). I don't like this solution because it requires extensive rewrite of all C code since all stack references would be unsafe.

### Regular expressions

Regular expressions is useful when parsing regular languages. However most RE libraries is implemented with bloated libraries that runs the compiled expressions in virtual machines. I never understood the point of this. We have a perfectly fine x86_64 machine so why would we want to run it in a virtual one? Also, when I say regular expressions I mean the academic (real) definition, not the Perl one with backtracking that has literally none of the intresting performance guarantees that real RE provides. Don't use RE to parse grammar that is not regular, the end.

First I looked at [re2c](http://re2c.org/) but I found it too archaic. For example, capturing parts of the match was not possible, so I wrote my own which is built into the librcd post preprocessor. It also compiles the pattern with [DFA](http://en.wikipedia.org/wiki/Deterministic_finite_automaton) which allows each unique state to be represented exactly once. This means that each character fed into the state automata moves it forward with exactly one jump to a new state. It's also possible to stream chunks to do streaming matching. Since it generates C code from the regex pattern you also get all the benefits of clang optimization on the fly. It's written in PHP and probably has a few bugs (like the fact that it's written in PHP). Open an issue if you find one.

Here's an example that validates a JSON number:

    static bool validate_number(fstr_t str) {
        #pragma re2c(str): ^ -? (0|[1-9][0-9]*) (\.[0-9]+)? ([eE][-\+]?[0-9]+)? $ {@return 1}
        return false;
    }


The "return 1" is a quirk that comes from the fact that the code is generated after preprocessing at a point where the `true` macro is no longer available. Anyway [you can find the code generated from the above pattern here](https://gist.github.com/hannes-landeholm/afd2caa13f5051c26eb0).

I should probably write a blog post which explains how streaming regex matching works sometime.

### DWARF and debug symbols for backtraces and log prints

Reflection is helpful for debugging purposes. Initially I relied on DWARF for this is librcd but soon realized how horribly inefficient the format was. To convert a single pointer to a file, line or function name you have to traverse a large number of non indexed debug chunks of dynamic size in a linear search. I therfore added a post linking stage in the librcd compile chain which utilizes a DWARF parser written in python. It parses and indexes the debug information and makes a complete sorted vector which maps address to file, line and function name and appends it as an ELF section. This allows lookups in O(log2(n)) time. An API to this information can be found in `reflect.h`. This allows librcd to print helpful backtraces and context information which is used constantly when debugging.

### "Librcd isn't ready for the web. It's not WebScale™."

The real world use case for librcd was to be used for different kind of web technologies. It already has built in json, rest and SSL/TLS support. In a few weeks we wrote our own web server and template language which allowed us to get rid of node.js. It will be open sourced as well. Stay tuned!

### Why librcd is not based on libc

In my opinion, libc has a lot of deprecated design that makes little sense in modern (high level) software development. Malloc and free forces people to resort to anti patterns such as performing max allocations on the stack to avoid the pains of manual memory management. Printf and scanf requires a completely new language that need to be interpreted in run time which is inefficient and makes static type checking in compile time pointless. They also need buffers managed for them which is prone to security problems. Libc encourages pointer arithmetics which is unsafe and hard to read. 

The string functions provided by libc is arbitrary and incomplete. Worst of all, libc cements the null terminated string concept. I would go as far as describing it as an anti pattern. It might save a few cycles on a bronze age computer if you ignore the fact that most programmers litter their code with strlen() calls. It could also save a few bytes that will be rounded by the heap allocator, but at the same time is kills compatibility with binary data, forces you to copy strings if you want to reference a substring and creates subtle off-by-one errors and hard to find security problems. All sensible string representations in other languages store the length together with the pointer.

### "I refuse to abandon libc!"

You know the part that said librcd had no libc? That was partly a lie. Librcd has a compatibility layer which is based on [musl](http://www.musl-libc.org/). A processing script is used to take a specific version and selection of the musl source code and process it into a single file. Other source code is used as well such as specific implementations of printf/scanf and librcd code that (inefficiently) ports calls to stdio and pthreads to librcd.

The glibc compatibility layer is useful when compiling existing C libraries with librcd. It is also used for some syscalls since the Linux API strongly assumes you are using glibc and depends on its structures and wrappers. In some cases Linux documents only the glibc interface and leaves the actual syscall interface undefined. You can also use the glibc layer yourself if you really, really like the 80s.

Librcd even have experimental support for pthreads! It's built on top of librcd concurrency. This is useful when compiling dependencies that use pthreads for synchronization. Be aware though that the performance is horrible.


### Why I created this

Some people will likely ask why I created this instead of "just using Go, Rust, etc". Essentially I had the problem of having to write highly concurrent and correct network software components while at the same time interfacing directly with the Linux kernel so I could use bleeding edge features. Betting on a language felt too risky at the time. Rust was crashing a lot and Go was still changing. I also felt really productive in C since I had a great IDE on my side (Netbeans) that gave me auto completion, symbol navigation etc. while the new languages had little to none editor/IDE support. Going with C allowed me to build something that worked from day 1 and then slowly fixing annoyances iteratively without getting locked in by design decisions made by the language gods.


### You can help

There is a lot of things I want to improve in librcd if I had the resources. You can help! (Simply go to the issues page and pick something intresting)[https://github.com/jumpstarter-io/librcd/issues]. Don't be afraid to send me a pull request if you want something changed or updated. In particular I'd really would like help in making the dependency and build system more community friendly, like being compatible with or even replaced by npm. You can also spread the word to people that might find librcd interesting.

Thanks for reading!
