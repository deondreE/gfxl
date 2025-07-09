@echo off
setlocal

:: Define file names
set ASM_FILE="output.s"
set OBJ_FILE="output.o"
set EXE="program.exe"
set PRINT_INT_C="print_int.c"
set PRINT_INT_OBJ="print_int.o"

:: Log files for warnings/errors
set AS_LOG="as_warnings.log"
set GCC_LOG="gcc_warnings.log"
set LD_LOG="ld_warnings.log"

echo.
echo [*] Assembling %ASM_FILE%...
:: as (assembler) from GNU Binutils (part of MinGW-w64)
:: -o for output object file
:: 2> for redirecting stderr (warnings/errors) to a log file
:: || (OR) operator: if 'as' fails (returns non-zero exit code), then execute the block
as -o %OBJ_FILE% %ASM_FILE% 2> %AS_LOG%
if %errorlevel% neq 0 (
    echo [!] Assembler error:
    type %AS_LOG%
    goto :cleanup
)

echo [*] Compiling %PRINT_INT_C%...
:: gcc (C compiler) from MinGW-w64
:: -c for compile only (don't link)
:: -o for output object file
:: 2> for stderr redirection
gcc -c -o %PRINT_INT_OBJ% %PRINT_INT_C% 2> %GCC_LOG%
if %errorlevel% neq 0 (
    echo [!] print_int.c compile error:
    type %GCC_LOG%
    goto :cleanup
)

echo [*] Linking...
:: ld (linker) from GNU Binutils (part of MinGW-w64)
:: -o for output executable
:: -L and -l for linking with standard C library (msvcrt.lib on MinGW)
:: For Windows, you typically link directly against the C runtime library (msvcrt.lib/msvcrt.dll)
:: MinGW GCC automatically handles the dynamic linker for Windows executables,
:: so you don't need `/lib64/ld-linux-x86-64.so.2` explicitly.
ld -o %EXE% %OBJ_FILE% %PRINT_INT_OBJ% -lmingw32 -lmsvcrt 2> %LD_LOG%
if %errorlevel% neq 0 (
    echo [!] Linker error:
    type %LD_LOG%
    goto :cleanup
)

:: Check for warnings in logs
echo.
echo [*] Checking for warnings...
for %%f in (%AS_LOG% %GCC_LOG% %LD_LOG%) do (
    findstr /i /c:"warning" %%f >nul
    if %errorlevel% equ 0 (
        echo [!] Warnings found in %%f:
        type %%f | findstr /i /c:"warning"
    )
)

echo.
echo [*] Running %EXE%...
%EXE%
set "EXIT_CODE=%errorlevel%"

echo.
if %EXIT_CODE% neq 0 (
    echo [!] Program exited with code %EXIT_CODE%
) else (
    echo [*] Program exited successfully.
)

:cleanup
echo.
echo [*] Cleaning up intermediate files...
del %OBJ_FILE% %PRINT_INT_OBJ% %AS_LOG% %GCC_LOG% %LD_LOG% 2>nul

endlocal
goto :eof