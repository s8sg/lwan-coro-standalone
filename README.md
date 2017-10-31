# lwan-coro-standalone
This is a direct copy of lawn CORO implementation with different Coroutine UseCase

Coro LIB Origin : https://github.com/lpereira/lwan/tree/master/src/lib

#### Build: 
```bash
gcc lwan-coro-example.c lwan-array.c lwan-coro.c -o example
```
#### Run:
```bash
$ ./example
coro_new coro1 and coro2
called coro_func_foo
yield in coro_func_foo
called coro_func_bar
yield in coro_func_bar
continue in coro_func_foo
continue in coro_func_bar
coro_free coro1 and coro2
```
