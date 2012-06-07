@echo ========================= MSGPACKALT BUILD SCRIPT =========================
@set MSGPACKDIR=../msgpack-0.5.4
cl /nologo /DMSGPACK_BUILDDLL /LD /Ox /O2 /W4 msgpackalt.c
@cd tests
cl /nologo /Ox /O2 /I.. test.c
cl /nologo /Ox /O2 /I.. /EHsc /Tp test.cpp /Fetestcpp
cl /nologo /Ox /O2 /I.. speed_test.c
cl /nologo /Ox /O2 /MD /I../%MSGPACKDIR%/src /Tp speed_test0.c /link ws2_32.lib ../%MSGPACKDIR%/lib/msgpack.lib
cl /nologo /Ox /O2 /MD /I../%MSGPACKDIR%/src /Tp test0.c /link ws2_32.lib ../%MSGPACKDIR%/lib/msgpack.lib
@del *.obj
@cd ..
@del *.obj
