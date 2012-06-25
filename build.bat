@echo ========================= MSGPACKALT BUILD SCRIPT =========================
@set MSGPACKDIR=../msgpack-0.5.4
cl /nologo /DMSGPACK_BUILDDLL /LD /Ox /O2 /W4 msgpackalt.c
@del *.obj
7z -mx9 u msgpackalt-msvc9.zip msgpackalt.c msgpackalt.dll msgpackalt.h msgpackalt.hpp msgpackalt.lib stdint_msc.h >NUL
