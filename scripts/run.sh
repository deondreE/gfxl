#!/bin/bash
set -euo pipefail

ASM_FILE="output.s"
OBJ_FILE="output.o"
EXE="program"
PRINT_INT_C="print_int.c"
PRINT_INT_OBJ="print_int.o"

echo "[*] Assembling $ASM_FILE..."
as -o "$OBJ_FILE" "$ASM_FILE" 2> as_warnings.log || {
    echo "[!] Assembler error:"
    cat as_warnings.log
    exit 1
}

echo "[*] Compiling $PRINT_INT_C..."
gcc -c -o "$PRINT_INT_OBJ" "$PRINT_INT_C" 2> gcc_warnings.log || {
    echo "[!] print_int.c compile error:"
    cat gcc_warnings.log
    exit 1
}

echo "[*] Linking..."
ld -o "$EXE" "$OBJ_FILE" "$PRINT_INT_OBJ" -lc -dynamic-linker /lib64/ld-linux-x86-64.so.2 2> ld_warnings.log || {
    echo "[!] Linker error:"
    cat ld_warnings.log
    exit 1
}

# Check for warnings in as/gcc/ld logs
for log in as_warnings.log gcc_warnings.log ld_warnings.log; do
    if grep -iq "warning" "$log"; then
        echo "[!] Warnings found in $log:"
        grep -i "warning" "$log"
    fi
done

echo "[*] Running $EXE..."
./"$EXE"
EXIT_CODE=$?

if [ $EXIT_CODE -ne 0 ]; then
    echo "[!] Program exited with code $EXIT_CODE"
else
    echo "[*] Program exited successfully."
fi

# Clean up intermediate files
rm -f "$OBJ_FILE" "$PRINT_INT_OBJ" as_warnings.log gcc_warnings.log ld_warnings.log
