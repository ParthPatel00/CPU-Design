/*
 * control.h -- Control Unit interface
 *
 * The control unit is the brain of the CPU. It runs the fetch/decode/execute
 * cycle, which is the fundamental loop that every CPU in existence follows:
 *
 *   FETCH   -- read the next instruction from memory at the address in PC
 *   DECODE  -- break the 16-bit instruction word into its fields (opcode, dst, src, immediate)
 *   EXECUTE -- carry out the operation (call the ALU, access memory via the bus, update PC)
 *
 * This cycle repeats forever until the CPU halts. HALT is encoded as a JMP
 * instruction with offset 0, which causes PC to jump to itself and loop forever.
 * The emulator detects this condition and stops the loop.
 *
 * Three run modes are provided:
 *   cpu_step        -- one cycle at a time (used internally and for testing)
 *   cpu_run         -- run silently until halt (used for Hello World and Fibonacci)
 *   cpu_run_verbose -- run with printed output after every cycle (used for the timer demo)
 */

#ifndef CONTROL_H
#define CONTROL_H

#include "cpu.h"

/*
 * cpu_step -- execute one fetch/decode/execute cycle
 *
 * Returns 1 if the CPU is still running, 0 if it has halted.
 * Halt is detected when a JMP instruction targets its own address (JMP 0).
 */
int cpu_step(CPU *cpu);

/*
 * cpu_run -- run until halt
 *
 * Calls cpu_step in a loop until it returns 0. No output is produced during
 * execution. Used for Hello World (which produces its own output via IO) and
 * for Fibonacci (where results are inspected via memory dump afterward).
 */
void cpu_run(CPU *cpu);

/*
 * cpu_run_verbose -- run with cycle-by-cycle output
 *
 * Before each cycle, prints the current PC and the raw instruction word being
 * fetched. After each cycle, prints the full CPU state (all registers and flags).
 * Used for the timer demo to show exactly what happens during each
 * Fetch / Compute / Store step.
 */
void cpu_run_verbose(CPU *cpu);

#endif /* CONTROL_H */
