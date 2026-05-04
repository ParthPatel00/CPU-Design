/*
 * cpu.h -- CPU state and definitions
 *
 * This file defines everything that describes the state of the CPU at any
 * point in time: its registers, the FLAGS register, and all opcode constants.
 *
 * Every other component (ALU, control unit, bus) receives a pointer to a CPU
 * struct so it can read and modify the CPU state.
 */

#ifndef CPU_H
#define CPU_H

#include <stdint.h>

/* -------------------------------------------------------------------------
 * FLAGS register bit masks
 *
 * The FLAGS register is a 16-bit value where only the lower 4 bits are used.
 * Each bit is set or cleared automatically after every ALU operation.
 *
 *   Bit 0 -- Z (Zero)     : the result of the last operation was exactly 0
 *   Bit 1 -- N (Negative) : bit 15 of the result is 1 (negative in signed math)
 *   Bit 2 -- C (Carry)    : an unsigned overflow occurred (result exceeded 0xFFFF)
 *   Bit 3 -- V (Overflow) : a signed overflow occurred (result wrapped the signed range)
 *
 * BEQ checks the Z flag. The others are available for inspection.
 * ------------------------------------------------------------------------- */
#define FLAG_Z  (1 << 0)
#define FLAG_N  (1 << 1)
#define FLAG_C  (1 << 2)
#define FLAG_V  (1 << 3)

/* -------------------------------------------------------------------------
 * Opcode constants
 *
 * Each opcode is 4 bits wide (stored in bits 15:12 of every instruction).
 * 4 bits allows 16 distinct opcodes, numbered 0x0 through 0xF.
 *
 * These constants are used in the control unit's decode switch statement and
 * in the ALU to identify which operation to perform.
 * ------------------------------------------------------------------------- */
#define OP_MOV   0x0   /* dst = src                            (register copy)      */
#define OP_MOVI  0x1   /* dst = imm16                          (2-word instruction) */
#define OP_LOAD  0x2   /* dst = mem[src]                       (load from memory)   */
#define OP_STORE 0x3   /* mem[dst] = src                       (store to memory)    */
#define OP_ADD   0x4   /* dst = dst + src                      (addition)           */
#define OP_ADDI  0x5   /* dst = dst + signed_imm6              (add immediate)      */
#define OP_SUB   0x6   /* dst = dst - src                      (subtraction)        */
#define OP_AND   0x7   /* dst = dst & src                      (bitwise AND)        */
#define OP_OR    0x8   /* dst = dst | src                      (bitwise OR)         */
#define OP_XOR   0x9   /* dst = dst ^ src                      (bitwise XOR)        */
#define OP_CMP   0xA   /* FLAGS = result of (dst - src)        (compare, no write)  */
#define OP_JMP   0xB   /* PC = PC + offset  OR  PC = reg       (unconditional jump) */
#define OP_BEQ   0xC   /* if Z==1: PC = PC + offset            (branch if equal)    */
#define OP_CALL  0xD   /* push PC, PC = src                    (call subroutine)    */
#define OP_PUSH  0xE   /* mem[--SP] = src                      (push to stack)      */
#define OP_POP   0xF   /* dst = mem[SP++]                      (pop from stack)     */

/* -------------------------------------------------------------------------
 * Stack pointer initial value
 *
 * The stack lives in the top of RAM (0x8000 - 0xFEFF) and grows downward.
 * SP starts at 0xFEFF. The first PUSH pre-decrements SP to 0xFEFE, then
 * writes there. The first POP reads from SP and post-increments back to 0xFEFF.
 * ------------------------------------------------------------------------- */
#define SP_INIT  0xFEFF

/* -------------------------------------------------------------------------
 * CPU struct
 *
 * This is the complete state of the CPU. Every component that needs to
 * read or modify CPU state receives a pointer to one of these.
 *
 *   R[0..7] -- general purpose registers, each 16 bits wide
 *   PC      -- Program Counter: address of the next instruction to fetch
 *   SP      -- Stack Pointer: address of the top item on the stack
 *   FLAGS   -- status bits updated after every ALU operation
 * ------------------------------------------------------------------------- */
typedef struct {
    uint16_t R[8];   /* General purpose registers R0 through R7 */
    uint16_t PC;     /* Program Counter                         */
    uint16_t SP;     /* Stack Pointer                           */
    uint16_t FLAGS;  /* Status flags: Z, N, C, V                */
} CPU;

/* Set all registers to 0, PC to 0x0000, SP to SP_INIT, FLAGS to 0. */
void cpu_init(CPU *cpu);

/* Print all register values and flag bits to stdout. Used by verbose mode. */
void cpu_print(const CPU *cpu);

#endif /* CPU_H */
