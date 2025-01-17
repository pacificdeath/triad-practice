@echo off
setlocal enabledelayedexpansion

set MAIN_C="./main.c"
set MAIN_O="./build/main.o"
set MAIN_EXE="./build/main.exe"

if not exist "build\" (
    mkdir "build"
)

set D="-g -DDEBUG"
set COMPILE_ONLY=0
set DEBUG=""
set GDB=0

for %%x in (%*) do (
    if "%%x"=="help" (
        goto :help
    ) else if "%%x"=="c" (
        set COMPILE_ONLY=1
    ) else if "%%x"=="d" (
        set DEBUG=%D%
    ) else if "%%x"=="g" (
        set GDB=1
        set DEBUG=%D%
    )
)

set DEBUG=%DEBUG:"=%

gcc -c ^
    !DEBUG! ^
    %MAIN_C% ^
    -o%MAIN_O% ^
    -I./raylib/include/
if not %errorlevel% equ 0 (
    echo compilation of %MAIN_C% failed
    goto :end
)

gcc ^
    %MAIN_O% ^
    -o%MAIN_EXE% ^
    -O0 ^
    -Wall ^
    -Wextra ^
    -Wconversion ^
    -std=c99 ^
    -L./raylib/lib/ ^
    -lraylib ^
    -lopengl32 ^
    -lgdi32 ^
    -lwinmm
if not %errorlevel% equ 0 (
    echo compilation of main.exe failed
    goto :end
)

if %errorlevel% equ 0 (
    if !COMPILE_ONLY!==1 (
        goto :end
    ) else if !GDB!==1 (
        gdb --args %MAIN_EXE%
    ) else (
        %MAIN_EXE%
    )
)
goto :end

:help
    echo %0 ^[Options^]
    echo Options:
    echo    help           this thing
    echo    c              compile only
    echo    d              enable debug
    echo    g              run gdb after compiliation
goto :end

:end