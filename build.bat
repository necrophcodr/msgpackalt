@echo ========================= MSGPACKALT BUILD SCRIPT =========================
cl /nologo /DMSGPACK_BUILDDLL /LD /Ox /O2 /W4 msgpackalt.c
@if not %ERRORLEVEL% equ 0 goto end
@del *.obj *.exp *.manifest
7z -mx9 u msgpackalt.zip msgpackalt.c msgpackalt.dll msgpackalt.h msgpackalt.hpp msgpackalt.lib stdint_msc.h examples\*.c* > NUL
@echo.
:end
