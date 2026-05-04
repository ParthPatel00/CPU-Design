/*
 * control.c -- Control Unit: fetch, decode, and execute
 *
 * This is the core of the emulator. cpu_step() implements one complete
 * fetch/decode/execute cycle according to the ISA defined in 01_ISA_Design.md.
 *
 * Instruction word layout (16 bits):
 *
 *   Standard (most instructions):
 *     [15:12] opcode   (4 bits)
 *     [11:9]  dst      (3 bits) -- destination register index
 *     [8:6]   src      (3 bits) -- source register index
 *     [5:0]   extra    (6 bits) -- signed immediate for ADDI, unused elsewhere
 *
 *   JMP (PC-relative, bit 11 = 0):
 *     [15:12] opcode   (4 bits)
 *     [11]    mode=0
 *     [10:0]  offset   (11-bit signed, relative to instruction address)
 *
 *   JMP (register-indirect, bit 11 = 1):
 *     [15:12] opcode   (4 bits)
 *     [11]    mode=1
 *     [8:6]   src      (3 bits) -- register holding the target address
 *
 *   BEQ:
 *     [15:12] opcode   (4 bits)
 *     [11:0]  offset   (12-bit signed, relative to instruction address)
 *
 *   MOVI (two words):
 *     Word 1: [15:12] opcode, [11:9] dst, rest unused
 *     Word 2: full 16-bit immediate value
 */

#include <stdio.h>
#include "control.h"
#include "cpu.h"
#include "alu.h"
#include "bus.h"

/*
 * sign_extend -- interpret a bit field as a signed integer
 *
 * When we extract a field from a 16-bit instruction (e.g., a 6-bit immediate
 * or an 11-bit offset), we get an unsigned value. But those fields represent
 * signed numbers in two's complement. This function extends the sign bit to
 * fill a full 16-bit signed integer.
 *
 * Example: a 6-bit value of 0b111111 = 63 unsigned.
 * As a 6-bit signed number, bit 5 is the sign bit and is set, so the true
 * value is -1. sign_extend(63, 6) returns -1 as int16_t.
 *
 * How it works:
 *   - Find the sign bit position: 1 << (bits - 1)
 *   - If that bit is set in 'value', OR in all the upper bits with 1s to
 *     extend the sign. (~0u << bits) creates a mask of all 1s above bit position.
 */
static int16_t sign_extend(uint16_t value, int bits) {
    uint16_t sign_bit = (uint16_t)(1 << (bits - 1));
    if (value & sign_bit)
        return (int16_t)(value | (uint16_t)(~0u << bits));
    return (int16_t)value;
}

/*
 * cpu_step -- one fetch/decode/execute cycle
 *
 * Returns 1 if execution should continue, 0 if the CPU has halted.
 */
int cpu_step(CPU *cpu) {

    /* =========================================================================
     * FETCH
     * Read the instruction word at the address stored in PC.
     * Save the instruction's own address (instr_addr) before incrementing PC,
     * because JMP and BEQ offsets are relative to the instruction's address.
     * ========================================================================= */
    uint16_t instr_addr = cpu->PC;
    uint16_t instr      = bus_read(cpu->PC);
    cpu->PC++;   /* advance past this instruction; MOVI will advance again below */

    /* =========================================================================
     * DECODE
     * Extract every possible field from the instruction word upfront.
     * Not all fields are used by every instruction -- only the relevant ones
     * are read during the execute stage.
     * ========================================================================= */

    /* Bits 15:12 -- opcode (which instruction this is) */
    uint8_t opcode = (instr >> 12) & 0xF;

    /* Bits 11:9 -- destination register index (0-7 for R0-R7) */
    uint8_t dst    = (instr >> 9) & 0x7;

    /* Bits 8:6 -- source register index (0-7 for R0-R7) */
    uint8_t src    = (instr >> 6) & 0x7;

    /* Bits 5:0 -- 6-bit signed immediate for ADDI (-32 to +31) */
    int16_t imm6 = sign_extend(instr & 0x3F, 6);

    /* Bits 10:0 -- 11-bit signed offset for JMP PC-relative (-1024 to +1023) */
    int16_t offset11 = sign_extend(instr & 0x7FF, 11);

    /* Bits 11:0 -- 12-bit signed offset for BEQ (-2048 to +2047) */
    int16_t offset12 = sign_extend(instr & 0xFFF, 12);

    /* Bit 11 -- JMP mode: 0 = PC-relative (nearby), 1 = register-indirect (far) */
    uint8_t jmp_mode = (instr >> 11) & 0x1;

    /* =========================================================================
     * EXECUTE
     * Carry out the operation identified by the opcode.
     * ========================================================================= */
    switch (opcode) {

        case OP_MOV:
            /*
             * Copy the value of the source register into the destination register.
             * Updates Z and N flags based on the value copied.
             * Example: MOV R2, R5  -->  R2 = R5
             */
            cpu->R[dst] = cpu->R[src];
            alu_set_flags(cpu, cpu->R[dst]);
            break;

        case OP_MOVI:
            /*
             * Load a full 16-bit immediate value into the destination register.
             * This is a 2-word instruction: the first word (already fetched) contains
             * the opcode and dst. The second word, at the current PC, holds the value.
             * PC is advanced a second time to skip past the immediate word.
             *
             * MOVI does not update FLAGS -- it is just a data load.
             *
             * Example: MOVI R0, #0xFF00  -->  R0 = 0xFF00
             */
            cpu->R[dst] = bus_read(cpu->PC);
            cpu->PC++;
            break;

        case OP_LOAD:
            /*
             * Load the 16-bit word at the memory address held in the source register.
             * Updates Z and N flags based on the value loaded -- this allows BEQ to
             * immediately detect a null terminator after loading a string character.
             *
             * Example: LOAD R2, [R0]  -->  R2 = mem[R0]
             */
            cpu->R[dst] = bus_read(cpu->R[src]);
            alu_set_flags(cpu, cpu->R[dst]);
            break;

        case OP_STORE:
            /*
             * Write the source register's value to the memory address in the destination
             * register. If the address is in the IO region (>= 0xFF00), the bus routes
             * this to the appropriate device instead of RAM.
             *
             * Example: STORE [R1], R2  -->  mem[R1] = R2
             */
            bus_write(cpu->R[dst], cpu->R[src]);
            break;

        case OP_ADD:
            /*
             * Add the source register to the destination register, store result in dst.
             * Updates all four flags (Z, N, C, V).
             *
             * Example: ADD R1, R2  -->  R1 = R1 + R2
             */
            cpu->R[dst] = alu_execute(cpu, OP_ADD, cpu->R[dst], cpu->R[src]);
            break;

        case OP_ADDI:
            /*
             * Add a signed 6-bit immediate (-32 to +31) to the destination register.
             * Negative immediates implement subtraction: ADDI R0, -1 decrements R0.
             * Updates all four flags (Z, N, C, V).
             *
             * The assembler converts:  SUBI R0, #1  -->  ADDI R0, #-1
             *
             * Example: ADDI R0, #-1  -->  R0 = R0 - 1
             */
            cpu->R[dst] = alu_execute(cpu, OP_ADDI, cpu->R[dst], (uint16_t)imm6);
            break;

        case OP_SUB:
            /*
             * Subtract the source register from the destination, store result in dst.
             * Updates all four flags (Z, N, C, V).
             *
             * Example: SUB R3, R4  -->  R3 = R3 - R4
             */
            cpu->R[dst] = alu_execute(cpu, OP_SUB, cpu->R[dst], cpu->R[src]);
            break;

        case OP_AND:
            /*
             * Bitwise AND of dst and src, result stored in dst.
             * Updates Z and N flags. C and V are cleared (no carry in AND).
             *
             * Example: AND R1, R2  -->  R1 = R1 & R2
             */
            cpu->R[dst] = alu_execute(cpu, OP_AND, cpu->R[dst], cpu->R[src]);
            break;

        case OP_OR:
            /*
             * Bitwise OR of dst and src, result stored in dst.
             * Updates Z and N flags.
             *
             * Example: OR R1, R2  -->  R1 = R1 | R2
             */
            cpu->R[dst] = alu_execute(cpu, OP_OR, cpu->R[dst], cpu->R[src]);
            break;

        case OP_XOR:
            /*
             * Bitwise XOR of dst and src, result stored in dst.
             * Updates Z and N flags. XOR of a register with itself always gives 0.
             *
             * Example: XOR R1, R2  -->  R1 = R1 ^ R2
             */
            cpu->R[dst] = alu_execute(cpu, OP_XOR, cpu->R[dst], cpu->R[src]);
            break;

        case OP_CMP:
            /*
             * Compare dst and src by computing (dst - src) and updating FLAGS.
             * The result is discarded -- neither register is changed.
             * Used before BEQ to compare two registers without destroying either.
             *
             * Example: CMP R3, R4  -->  FLAGS = result of (R3 - R4), R3 unchanged
             */
            alu_execute(cpu, OP_CMP, cpu->R[dst], cpu->R[src]);
            break;

        case OP_JMP:
            /*
             * Unconditional jump. Two forms determined by bit 11 of the instruction:
             *
             * PC-relative (bit 11 = 0):
             *   Jump by a signed 11-bit offset from this instruction's address.
             *   Used for nearby jumps like looping back to the top of a loop.
             *   HALT is encoded as JMP with offset 0 (jumps to itself forever).
             *
             * Register-indirect (bit 11 = 1):
             *   Jump to the address stored in the src register.
             *   Used for far jumps -- load the target address with MOVI first,
             *   then JMP to that register. Reaches any address in the 64 KB space.
             *
             * Example (PC-relative): JMP -3     -->  PC = PC - 3
             * Example (register):    JMP R7     -->  PC = R7
             */
            if (jmp_mode == 0)
                cpu->PC = (uint16_t)(instr_addr + offset11);  /* PC-relative */
            else
                cpu->PC = cpu->R[src];                         /* register-indirect */
            break;

        case OP_BEQ:
            /*
             * Branch if Equal: jump only if the Z (zero) flag is set.
             * Z is set by the previous ALU instruction (ADD, ADDI, SUB, CMP, LOAD, etc.)
             * when its result was exactly zero.
             *
             * Uses a 12-bit signed PC-relative offset (relative to this instruction's
             * address), giving a range of -2048 to +2047 words. Conditional branches
             * are always short (loop tops, if-exits), so this is sufficient.
             *
             * Example: after CMP R0, R1 sets Z=1:
             *   BEQ done  -->  PC = instr_addr + offset  (jumps to 'done')
             */
            if (cpu->FLAGS & FLAG_Z)
                cpu->PC = (uint16_t)(instr_addr + offset12);
            break;

        case OP_CALL:
            /*
             * Call a subroutine at the address held in the src register.
             *
             * Steps:
             *   1. Decrement SP (stack grows downward: --SP before writing)
             *   2. Push the current PC (return address) onto the stack
             *   3. Set PC to the target address in src
             *
             * To return from the subroutine, the assembler expands RET into:
             *   POP R7    -- retrieve return address into R7
             *   JMP R7    -- jump back to the caller
             *
             * The target address must be pre-loaded with MOVI:
             *   MOVI R6, #my_function
             *   CALL R6
             */
            cpu->SP--;
            bus_write(cpu->SP, cpu->PC);  /* save return address to stack */
            cpu->PC = cpu->R[src];         /* jump to subroutine           */
            break;

        case OP_PUSH:
            /*
             * Push the source register onto the stack.
             * SP is pre-decremented before writing so the stack grows downward.
             *
             * Example: PUSH R5  -->  SP--; mem[SP] = R5
             */
            cpu->SP--;
            bus_write(cpu->SP, cpu->R[src]);
            break;

        case OP_POP:
            /*
             * Pop the top of the stack into the destination register.
             * SP is post-incremented after reading so it moves back up.
             *
             * Example: POP R5  -->  R5 = mem[SP]; SP++
             *
             * RET (pseudo-instruction) expands to: POP R7 followed by JMP R7
             */
            cpu->R[dst] = bus_read(cpu->SP);
            cpu->SP++;
            break;
    }

    /* =========================================================================
     * HALT DETECTION
     *
     * HALT is the assembler pseudo-instruction that assembles to JMP with
     * offset 0. After executing, PC = instr_addr + 0 = instr_addr, meaning
     * the CPU is about to re-fetch the same JMP instruction forever.
     * We detect this and return 0 to stop the run loop.
     * ========================================================================= */
    if (opcode == OP_JMP && jmp_mode == 0 && cpu->PC == instr_addr)
        return 0;  /* halted */

    return 1;  /* still running */
}

/*
 * cpu_run -- run the program silently until halt
 *
 * Repeatedly calls cpu_step until the CPU halts. No output during execution.
 * The program produces its own output via memory-mapped IO (Hello World, Fibonacci).
 */
void cpu_run(CPU *cpu) {
    while (cpu_step(cpu));
}

/*
 * cpu_run_verbose -- run with cycle-by-cycle output for each F/C/S step
 *
 * Before each cycle, prints the FETCH info (PC and raw instruction word).
 * After the DECODE header, calls cpu_step to execute.
 * After each cycle, prints the full CPU state showing what changed.
 *
 * This is the mode used for the Timer demo, where the goal is to show
 * the Fetch / Compute / Store stages happening step by step.
 */
void cpu_run_verbose(CPU *cpu) {
    uint32_t cycle = 0;
    int running = 1;

    while (running) {
        printf("\n========= Cycle %u =========\n", cycle++);

        /* FETCH: show what is about to be read and executed */
        uint16_t instr = bus_read(cpu->PC);
        printf("[FETCH]   PC=0x%04X  instruction=0x%04X\n", cpu->PC, instr);

        /* DECODE: show the opcode and register fields */
        printf("[DECODE]  opcode=0x%X  dst=R%u  src=R%u\n",
            (instr >> 12) & 0xF,
            (instr >> 9)  & 0x7,
            (instr >> 6)  & 0x7);

        /* EXECUTE: run the instruction, then print the updated CPU state */
        running = cpu_step(cpu);
        printf("[EXECUTE] ");
        cpu_print(cpu);
    }

    printf("\n[HALTED]\n");
}
