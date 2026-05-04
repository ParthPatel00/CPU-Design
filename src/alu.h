/*
 * alu.h -- Arithmetic Logic Unit interface
 *
 * The ALU is the part of the CPU that performs all math and logic operations.
 * It takes two 16-bit input values, performs the requested operation, updates
 * the FLAGS register to reflect the result, and returns the 16-bit output.
 *
 * Operations handled here: ADD, ADDI, SUB, CMP, AND, OR, XOR
 *
 * MOV and LOAD also update flags (they set Z and N based on the value moved
 * or loaded), but they do not go through alu_execute. Instead they call
 * alu_set_flags directly since they have no second operand to compute with.
 *
 * STORE, JMP, BEQ, CALL, PUSH, POP do not affect flags at all.
 */

#ifndef ALU_H
#define ALU_H

#include <stdint.h>
#include "cpu.h"

/*
 * alu_execute -- perform an ALU operation and update FLAGS
 *
 * Parameters:
 *   cpu  -- pointer to CPU state (FLAGS will be updated)
 *   op   -- the opcode: OP_ADD, OP_ADDI, OP_SUB, OP_CMP, OP_AND, OP_OR, OP_XOR
 *   a    -- first operand (always the destination register's current value)
 *   b    -- second operand (source register value, or sign-extended immediate)
 *
 * Returns the 16-bit result. For CMP, the result is computed and FLAGS are set
 * but the caller discards the return value (result is not written back).
 */
uint16_t alu_execute(CPU *cpu, uint8_t op, uint16_t a, uint16_t b);

/*
 * alu_set_flags -- set Z and N flags based on a value, clear C and V
 *
 * Used by MOV and LOAD after they produce a result without performing arithmetic.
 * C and V only have meaning for addition and subtraction, so they are cleared.
 */
void alu_set_flags(CPU *cpu, uint16_t result);

#endif /* ALU_H */
