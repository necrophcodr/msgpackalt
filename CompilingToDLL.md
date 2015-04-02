Here are instructions for building a dynamically linked library (DLL) for Windows for both the original Message Pack and msgpackalt implementations.


## Compiling msgpackalt to DLL ##
Because this was one of the original design goals, it is extremely simple to do. Simply define the preprocessor macro `MSGPACK_BUILDDLL` and compile `msgpackalt.c` as a shared object:

| **Compiler** | **Example syntax** |
|:-------------|:-------------------|
| MingW GCC | `gcc -shared -o msgpackalt.dll msgpackalt.c -D MSGPACK_BUILDDLL -Wall -O3 -Wl,--out-implib,libmsgpackalt.a` |
| MS Visual-C | `cl /nologo /DMSGPACK_BUILDDLL /LD /Ox /O2 /W4 msgpackalt.c` |

Because msgpackalt is so small, it is not envisaged that C/C++ programs will want to dynamically link to it. Therefore no DLL import functionality is provided in the headers. This can be easily changed if there is desire to do so, but compiling the single `msgpackalt.c` file into your program and inlining seems substantially less effort and more efficient.



## Compiling Message Pack to DLL ##
Unfortunately in this case it's not as simple as just executing the linker because many of the functions are both _inline_ and _static_, so they're not accessible outside the object they're defined in. Because they are **explicitly** defined as such (despite being a standard compiler optimisation), we have to trick the compiler into doing the opposite of what it has been told and extern them instead.

By comparison, it's easier to make a _static_ linked library from the command line than using the VC project file:
```
cl /nologo /EHsc /c /TP src/*.c src/*.cpp
lib /nologo *.obj /out:lib/msgpack.lib
```


### Creating a module definiton ###
The [module definition](http://msdn.microsoft.com/en-us/library/28d6s79h.aspx) file tells the linker explicitly which objects to provide access to in the DLL. It has a [simple text-based format](http://msdn.microsoft.com/en-us/library/hyx1zcd3.aspx) listing the function names.

The simplest way to scrub all the function definitions from the header files is to get GCC to pre-process them and output them in a standard format (solving text-formatting problems at the very least). This is then processed by some standard GNU tools to create the DEF:
```
gcc msgpack.h -aux-info proto.tmp -I .
echo -e "LIBRARY\tMSGPACK\nEXPORTS" > msgpack.def
sed -nr 's/^.+(msgpack_[a-z0-9_]+) \(.*$/  \1/p' proto.tmp | awk '!($0 in a) {a[$0];print}' >> msgpack.def
```

This is a complete list of all msgpack functions.


### Externing the functions ###
Now to get around the inline/static function definition problem, we'll create a new source file that forces the functions to be exported. We'll use the same approach as above but create instances of the static functions only, and use a nasty preprocessor hack to eliminate the inline.

Note that GCC will mangle the bool type name so we have to correct for it.

```
echo -e '#define static\n#include "msgpack.h"' > static.c
sed -nr 's/^.+ static ([[:alnum:]_ ]+\*?msgpack_[a-z0-9_]+ \([^;]+).*$/extern \1;/p' proto.tmp | sed 's/_Bool/bool/' >> static.c
```


### Compiling ###
We can now create a DLL. Instead of modifying the project file, it's much easier to simply call Visual-C++ from the base directory:
```
cl /Ox /FD /MD /W2 /TP /LD /Felib\msgpack.dll src\*.c /link /DEF:src\msgpack.def ws2_32.lib
```

Which will produce a shiny DLL for you, with the result easily demonstrated in Python:
```
>>> import ctypes as C
>>> lib = C.WinDLL('msgpack.dll')
>>> print C.string_at(lib.msgpack_version())
0.5.4
```

All the functions are there, but the design of Message Pack requires the use of function pointers, which some languages (eg LabView) do not support, and for which work-arounds are more complicated than _[reimplementation](why.md)_.