@echo ========================= MSGPACKALT BUILD SCRIPT =========================
@set MSGPACKDIR=../msgpack-0.5.4
cl /nologo /DMSGPACK_BUILDDLL /LD /Ox /O2 /W4 msgpackalt.c
@del *.obj
