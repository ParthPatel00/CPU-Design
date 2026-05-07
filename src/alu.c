/*
 * alu.c -- Arithmetic Logic Unit implementation
 *
 * All arithmetic and logic operations are performed here. After every
 * operation the FLAGS register is updated so the control unit can use
 * BEQ to branch based on the result.
 *
 * Flag update rules:
 *   Z -- set if result == 0
 *   N -- set if bit 15 of result is 1 (the sign bit in 16-bit signed math)
 *   C -- set if the true mathematical result exceeded 0xFFFF (unsigned carry)
 *   V -- set if a signed overflow occurred (positive + positive = negative, etc.)
 */

#include "alu.h"

/*
 * alu_set_flags -- update Z and N only, clear C and V
 *
 * Used for operations that produce a value but have no concept of carry or
 * overflow: MOV, LOAD, AND, OR, XOR.
 *
 * Always clears FLAGS to 0 first so stale bits from a previous instruction
 * do not linger.
 */
void alu_set_flags(CPU *cpu, uint16_t result) {
    cpu->FLAGS = 0;
    if (result == 0)     cpu->FLAGS |= FLAG_Z;  /* result is zero     */
    if (result & 0x8000) cpu->FLAGS |= FLAG_N;  /* bit 15 is set      */
}

/*
 * set_flags_arith -- update all four flags after addition or subtraction
 *
 * Parameters:
 *   result  -- the 16-bit answer (already cut down to 16 bits)
 *   full    -- the same answer computed in 32 bits (wider, for carry check)
 *   a, b    -- the two original values before the operation
 *   is_sub  -- pass 1 for subtraction, 0 for addition
 *
 * How each flag is set:
 *
 *   Z (zero):    result is exactly 0.
 *
 *   N (negative): the top bit of the result is 1, which in signed math
 *                 means the number is negative.
 *
 *   C (carry):   the true answer was bigger than 65535 and didn't fit in
 *                16 bits. We catch this by computing in 32 bits and checking
 *                if anything ended up above 0xFFFF.
 *
 *   V (overflow): the answer is wrong in signed arithmetic. This happens
 *                 when you add two positive numbers and the result looks
 *                 negative, or add two negatives and get a positive.
 *                 Example: 32767 + 1 = 32768, but in 16-bit signed that
 *                 wraps to -32768, which is clearly wrong.
 *                 For subtraction the same idea applies in reverse.
 */
static void set_flags_arith(CPU *cpu, uint16_t result, uint32_t full,
                             uint16_t a, uint16_t b, int is_sub) {
    uint16_t sign_a, sign_b, sign_r;

    cpu->FLAGS = 0;

    if (result == 0)     cpu->FLAGS |= FLAG_Z;
    if (result & 0x8000) cpu->FLAGS |= FLAG_N;
    if (full > 0xFFFF)   cpu->FLAGS |= FLAG_C;  /* carry: result did not fit in 16 bits */

    /* Extract the sign bits of each operand and the result */
    sign_a = a & 0x8000;
    sign_b = b & 0x8000;
    sign_r = result & 0x8000;

    /* Signed overflow for addition: same-sign inputs, different-sign output */
    if (!is_sub && (sign_a == sign_b) && (sign_r != sign_a))
        cpu->FLAGS |= FLAG_V;

    /* Signed overflow for subtraction: different-sign inputs, result differs from a */
    if (is_sub && (sign_a != sign_b) && (sign_r != sign_a))
        cpu->FLAGS |= FLAG_V;
}

/*
 * alu_execute -- run an ALU operation and return the result
 *
 * The control unit calls this for every arithmetic or logic instruction.
 * It handles the computation, updates FLAGS, and returns the 16-bit result.
 *
 * ADD and ADDI share the same addition logic -- the only difference is that
 * ADDI's second operand is a sign-extended immediate rather than a register.
 * The control unit handles that sign extension before calling here.
 *
 * SUB and CMP share the same subtraction logic. For CMP, the control unit
 * discards the returned result so FLAGS are set but no register is changed.
 *
 * Subtraction is implemented as two's complement addition:
 *   a - b = a + (~b) + 1
 * This naturally produces the correct carry flag for unsigned borrow detection.
 */
uint16_t alu_execute(CPU *cpu, uint8_t op, uint16_t a, uint16_t b) {
    uint32_t full;
    uint16_t result;

    switch (op) {

        case OP_ADD:
        case OP_ADDI:
            full   = (uint32_t)a + (uint32_t)b;
            result = (uint16_t)full;
            set_flags_arith(cpu, result, full, a, b, 0 /* not subtraction */);
            return result;

        case OP_SUB:
        case OP_CMP:
            /* Two's complement subtraction: a - b = a + (~b) + 1 */
            full   = (uint32_t)a + (uint32_t)(~b) + 1;
            result = (uint16_t)full;
            set_flags_arith(cpu, result, full, a, b, 1 /* is subtraction */);
            return result;   /* caller discards this for CMP */

        case OP_AND:
            result = a & b;
            alu_set_flags(cpu, result);
            return result;

        case OP_OR:
            result = a | b;
            alu_set_flags(cpu, result);
            return result;

        case OP_XOR:
            result = a ^ b;
            alu_set_flags(cpu, result);
            return result;

        default:
            return 0;
    }
}
