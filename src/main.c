/*
 * main.c -- Emulator entry point
 *
 * This is the program you run from the command line. It accepts a binary file
 * produced by the assembler, loads it into memory, and runs it.
 *
 * Three modes are available:
 *
 *   run <file.bin>
 *     Load and run the program silently. Output appears only from IO writes
 *     (characters printed by Hello World, numbers printed by Fibonacci).
 *
 *   verbose <file.bin>
 *     Load and run with full cycle-by-cycle output. Before each instruction,
 *     the FETCH and DECODE info is printed. After each instruction, the full
 *     CPU state (all registers and flags) is printed. Used for the timer demo
 *     to demonstrate the Fetch / Compute / Store cycle visually.
 *
 *   dump <file.bin> <from> <to>
 *     Run the program silently, then print the contents of memory between
 *     address 'from' and address 'to'. Used after Fibonacci to inspect the
 *     sequence stored in RAM. Addresses can be given in hex (0x8000) or decimal.
 *
 * Usage examples:
 *   ./cpu run     hello.bin
 *   ./cpu verbose timer.bin
 *   ./cpu dump    fib.bin 0x8000 0x8009
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "memory.h"
#include "control.h"

/* Print usage instructions to stdout. */
static void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s run     <file.bin>                  Run a program\n", prog);
    printf("  %s verbose <file.bin>                  Run with cycle-by-cycle output\n", prog);
    printf("  %s dump    <file.bin> <from> <to>      Run then dump a memory range\n", prog);
    printf("\nAddresses for dump can be hex (0x8000) or decimal.\n");
}

int main(int argc, char *argv[]) {

    /* Need at least a mode and a filename */
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    /* Initialize memory to all zeros and reset the CPU to its start state */
    memory_init();
    CPU cpu;
    cpu_init(&cpu);

    /* Load the binary program file into memory starting at address 0x0000 */
    if (memory_load_file(argv[2]) != 0)
        return 1;   /* memory_load_file already printed the error */

    /* Dispatch to the correct run mode based on the first argument */
    if (strcmp(argv[1], "run") == 0) {
        /*
         * Silent run: execute until halt.
         * Program output appears via memory-mapped IO during execution.
         */
        cpu_run(&cpu);

    } else if (strcmp(argv[1], "verbose") == 0) {
        /*
         * Verbose run: print FETCH, DECODE, and full CPU state each cycle.
         * Used for the timer demo to show Fetch / Compute / Store stages.
         */
        cpu_run_verbose(&cpu);

    } else if (strcmp(argv[1], "dump") == 0) {
        /*
         * Dump mode: run the program first, then print the requested memory range.
         * 'from' and 'to' are the start and end addresses (inclusive).
         * strtol with base 0 handles both "0x8000" (hex) and "32768" (decimal).
         */
        if (argc < 5) {
            printf("Error: dump requires <from> and <to> address arguments.\n");
            print_usage(argv[0]);
            return 1;
        }
        uint16_t from = (uint16_t)strtol(argv[3], NULL, 0);
        uint16_t to   = (uint16_t)strtol(argv[4], NULL, 0);
        cpu_run(&cpu);
        memory_dump(from, to);

    } else {
        printf("Error: unknown mode '%s'\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
