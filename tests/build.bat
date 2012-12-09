@echo ========================= MSGPACKALT BUILD SCRIPT =========================
@set MSGPACKDIR=\dev\libs\msgpack-0.5.7
cl /nologo /Ox /O2 /I.. test.c
cl /nologo /Ox /O2 /I.. /EHsc /Tp test.cpp /Fetestcpp
cl /nologo /Ox /O2 /I.. speed_test.c
@if not exist %MSGPACKDIR% goto done
cl /nologo /Ox /O2 /I %MSGPACKDIR%\src /Tp speed_test0.c /link ws2_32.lib %MSGPACKDIR%\lib\msgpack.lib
cl /nologo /Ox /O2 /I %MSGPACKDIR%\src /Tp test0.c /link ws2_32.lib %MSGPACKDIR%\lib\msgpack.lib
:done
@del *.obj 2>NUL
