@echo off

set "main_c=.\main.c"
set "raylib_dir=.\raylib"
set "build_dir=.\build"
set "main_exe=%build_dir%\main.exe"

if not exist "%build_dir%" mkdir "%build_dir%"

set "debug="
set "gdb="
set "gdb_init=.\gdb-init.txt"

for %%x in (%*) do (
    if "%%x"=="gdb" (
        set gdb=1
        set "debug=-g -DDEBUG"
    ) else if "%%x"=="debug" (
        set "debug=-g -DDEBUG"
    )
)

echo compiling...

gcc %debug% %main_c% -o%main_exe% ^
    -I%raylib_dir%\include\ ^
    -L%raylib_dir%\lib\ ^
    -lraylib -lopengl32 -lgdi32 -lwinmm

if errorlevel 1 (
    echo compilation failed horribly
    exit /b
)

echo compilation did not fail

if "%gdb%" equ "1" (
    gdb --command "%gdb_init%" --args "%main_exe%"
    exit /b
)

"%main_exe%"


