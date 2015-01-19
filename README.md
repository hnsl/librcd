# librcd
*A library for Rapid {C|Concurrency|Correct} Development*

Librcd is a new C standard library and POSIX replacement for x86_64 Linux, used for most things at [Jumpstarter](https://jumpstarter.io/). It comes with some custom syntax, and makes threading, strings, error handling, memory management and IO easy. Here's a small example:

```c
#include "rcd.h"
#pragma librcd

join_locked(int) switcheroo(int next, join_server_params, int* current) {
  int prev = *current;
  DBG("swapping ", prev, " <-> ", next);
  *current = next;
  return prev;
}

fiber_main worker(fiber_main_attr, int value, rcd_fid_t fid) {
  for (;;) {
    rio_wait(prng_rand() % RIO_NS_SEC);
    value = switcheroo(value, fid);
  }
}

void rcd_main(list(fstr_t)* main_args, list(fstr_t)* main_env) {
  for (int i = 1; i <= 5; i++) {
    fmitosis {
      spawn_fiber(worker("", i, rcd_self));
    }
  }
  int free_value = 0;
  auto_accept_join(switcheroo, join_server_params, &free_value);
}
```

For more code examples, covering e.g. strings, memory management and exceptions, see Hannes's blog post. You can also check out the source code of librcd (it's rather readable), or [wsr](https://github.com/jumpstarter-io/wsr), a basic web server library written on top of librcd.

Librcd is **highly non-portable**, and meant to be used mainly for server development, in a controlled environment. Only x86_64 Linux is supported, and this is [unlikely to change](https://github.com/jumpstarter-io/librcd/issues/2). Furthermore, due to license conflicts, binaries compiled with librcd [cannot currently be legally distributed](https://github.com/jumpstarter-io/librcd/issues/1).

Some documentation [can be found in the wiki](https://github.com/jumpstarter-io/librcd/wiki); see in particular the [Installing](https://github.com/jumpstarter-io/librcd/wiki/Installing) page. Make sure also to check out the mentioned blog post - it provides a lot of motivation and in-depth descriptions of most library features.
