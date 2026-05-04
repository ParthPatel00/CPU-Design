/*
 * cpu.c -- CPU initialization and state display
 *
 * Implements the two utility functions declared in cpu.h:
 *   cpu_init  -- resets the CPU to its power-on state
 *   cpu_print -- prints all register values to stdout
 *
 * The actual execution logic lives in control.c.
 * The flag update logic lives in alu.c.
 */

#include <stdio.h>
#include "cpu.h"

/*
 * cpu_init -- reset the CPU to its starting state
 *
 * Called once before running any program. Sets every general-purpose register
 * to 0, places PC at the start of program memory (0x0000), positions the stack
 * pointer at the top of RAM, and clears all flags.
 *
 * Programs are always loaded starting at address 0x0000, so PC = 0 means
 * the first instruction fetched will be the first instruction of the program.
 */
void cpu_init(CPU *cpu) {
    int i;
    for (i = 0; i < 8; i++)
        cpu->R[i] = 0;

    cpu->PC    = 0x0000; /* execution starts at the beginning of program memory */
    cpu->SP    = SP_INIT; /* stack starts at top of RAM, grows downward          */
    cpu->FLAGS = 0;       /* no flags set at startup                              */
}

/*
 * cpu_print -- display the full CPU state to stdout
 *
 * Prints each general-purpose register in both hex and unsigned decimal,
 * then PC, SP, and each individual flag bit. Called after every cycle in
 * verbose/timer mode so you can watch how the CPU state changes step by step.
 *
 * Example output:
 *   --- CPU State ---
 *     R0 = 0x0064 (100)
 *     R1 = 0xFF02 (65282)
 *     ...
 *     PC    = 0x0006
 *     SP    = 0xFEFF
 *     FLAGS = Z:0 N:0 C:0 V:0
 */
void cpu_print(const CPU *cpu) {
    int i;
    printf("--- CPU State ---\n");
    for (i = 0; i < 8; i++)
        printf("  R%d = 0x%04X (%u)\n", i, cpu->R[i], cpu->R[i]);
    printf("  PC    = 0x%04X\n", cpu->PC);
    printf("  SP    = 0x%04X\n", cpu->SP);
    printf("  FLAGS = Z:%d N:%d C:%d V:%d\n",
        (cpu->FLAGS >> 0) & 1,   /* Z: zero flag     */
        (cpu->FLAGS >> 1) & 1,   /* N: negative flag */
        (cpu->FLAGS >> 2) & 1,   /* C: carry flag    */
        (cpu->FLAGS >> 3) & 1);  /* V: overflow flag */
}
