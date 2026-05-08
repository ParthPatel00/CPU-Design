/*
 * asm_main.c -- Assembler entry point
 *
 * Reads an assembly source file and produces a binary file.
 *
 * Usage:
 *   ./asm  <input.asm>  <output.bin>
 *
 * Example:
 *   ./asm programs/hello.asm programs/hello.bin
 *   ./cpu run programs/hello.bin
 */

#include <stdio.h>
#include "assembler.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input.asm> <output.bin>\n", argv[0]);
        return 1;
    }

    Assembler as;
    asm_init(&as);

    if (asm_assemble(&as, argv[1], argv[2]) != 0)
        return 1;

    return 0;
}
