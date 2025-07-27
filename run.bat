@echo off
setlocal enabledelayedexpansion

set ENGINE_C="./src/engine.c"
set WINWRAPPER_C="./src/winwrapper.c"
set MAIN_C="./src/main.c"

set ENGINE_EXE="./build/engine.exe"
set MAIN_DLL="./build/main.dll"

set RAYLIB_DIR="./raylib"

if not exist "build\" (
    mkdir "build"
)

set D="-g -DDEBUG"
set START=0
set MAIN=0
set ENGINE=0
set DEBUG=""
set GDB=0
set COMPILE_ONLY=0
set DLL_LAST_MODIFIED=""
set GDB_INIT="./gdb-init.txt"

for %%x in (%*) do (
    if "%%x"=="help" (
        goto :help
    ) else if "%%x"=="start" (
        set START=1
    ) else if "%%x"=="main" (
        set MAIN=1
    ) else if "%%x"=="engine" (
        set ENGINE=1
    ) else if "%%x"=="debug" (
        set DEBUG=%D%
    ) else if "%%x"=="gdb" (
        set GDB=1
        set DEBUG=%D%
    )
)

set DEBUG=%DEBUG:"=%

if %MAIN% equ 1 (
    echo Compiling main DLL...
    gcc -shared -o %MAIN_DLL% %MAIN_C% %DEBUG% -I%RAYLIB_DIR%/include/ -L%RAYLIB_DIR%/lib/ -lraylib -lopengl32 -lgdi32 -lwinmm
    if not !errorlevel! equ 0 (
        echo Compilation of main.dll failed.
        goto :end
    )
    echo main DLL compiled successfully.
)

if %ENGINE% equ 1 (
    echo Compiling engine...
    gcc -c %DEBUG% %WINWRAPPER_C% -o ./build/winwrapper.o -I%RAYLIB_DIR%/include/

    if not !errorlevel! equ 0 (
        echo Compilation of winwrapper.o failed.
        goto :end
    )

    gcc -c %DEBUG% %ENGINE_C% -o ./build/engine.o -I%RAYLIB_DIR%/include/

    if not !errorlevel! equ 0 (
        echo Compilation of engine.o failed.
        goto :end
    )

    gcc ./build/engine.o ./build/winwrapper.o -o %ENGINE_EXE% -I%RAYLIB_DIR%/include/ -L%RAYLIB_DIR%/lib/ -lraylib -lopengl32 -lgdi32 -lwinmm

    if not !errorlevel! equ 0 (
        echo Linking engine.exe failed.
        goto :end
    )

    echo Engine compiled and linked successfully.
)

if %errorlevel% equ 0 (
    if %GDB% equ 1 (
        gdb --command %GDB_INIT% --args %ENGINE_EXE%
    ) else if %START% equ 1 (
        %ENGINE_EXE%
    )
)

:end
goto :eof

:help
    echo %0 [Options]
    echo Options:
    echo    help            Show this help
    echo    main            Compile main DLL
    echo    engine          Compile engine
    echo    start           Start engine
    echo    debug           Enable debug flags
    echo    gdb             Run gdb after compilation
goto :end

