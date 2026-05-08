/*
 * assembler.c -- Two-pass assembler for EduCore16
 *
 * Reads a human-readable .asm source file and writes a .bin binary file
 * that the CPU emulator can load and run directly.
 *
 * Why two passes?
 *   In a single pass, if you see "JMP done" before "done:" is defined,
 *   you don't yet know the address of "done". Pass 1 solves this by
 *   scanning the whole file first to collect all label addresses.
 *   Pass 2 then encodes every instruction, looking up labels as needed.
 *
 * What the assembler handles:
 *
 *   Labels         loop:              A name you can JMP or BEQ to.
 *   Registers      R0 - R7            Case-insensitive (R0 or r0 both work).
 *   Immediates     #10  #-3  #0xFF   A number after the # sign.
 *   Char literals  'A'                ASCII value 65, only valid in .word.
 *   Comments       ; anything         Everything after ; is ignored.
 *   Directives     .word v1, v2, ...  Write raw numbers directly into memory.
 *
 * Instruction encoding (matches docs/CPU_Design.md):
 *
 *   Most instructions: [opcode:4][dst register:3][src register:3][unused:6]
 *   ADDI:              [0101][dst:3][000][6-bit signed immediate]
 *   MOVI (2 words):    Word 1: [0001][dst:3][000000000]   Word 2: the value
 *   JMP / BEQ:         PC-relative signed offsets stored in lower bits
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "assembler.h"
#include "cpu.h"

/* -----------------------------------------------------------------------
 * Utility helpers
 * ----------------------------------------------------------------------- */

/* Case-insensitive string comparison. */
static int streqi(const char *a, const char *b) {
    return strcasecmp(a, b) == 0;
}

/* -----------------------------------------------------------------------
 * Symbol table
 * ----------------------------------------------------------------------- */

static void add_label(Assembler *as, const char *name, uint16_t addr) {
    if (as->label_count >= MAX_LABELS) {
        fprintf(stderr, "Error: too many labels (max %d)\n", MAX_LABELS);
        return;
    }
    strncpy(as->labels[as->label_count].name, name, 63);
    as->labels[as->label_count].name[63] = '\0';
    as->labels[as->label_count].address = addr;
    as->label_count++;
}

/* Return the address for a label, or -1 if not found. */
static int find_label(Assembler *as, const char *name) {
    for (int i = 0; i < as->label_count; i++) {
        if (strcmp(as->labels[i].name, name) == 0)
            return as->labels[i].address;
    }
    return -1;
}

/* -----------------------------------------------------------------------
 * Operand parsing
 * ----------------------------------------------------------------------- */

/* Parse a register name ("R0"-"R7", case-insensitive). Returns 0-7 or -1. */
static int parse_register(const char *s) {
    if ((s[0] == 'R' || s[0] == 'r') && s[1] >= '0' && s[1] <= '7' && s[2] == '\0')
        return s[1] - '0';
    return -1;
}

/*
 * Parse a numeric value from a string. Supports:
 *   decimal:  123, -5
 *   hex:      0xFF, 0x1A
 *   char:     'A' (returns ASCII value)
 */
static int parse_number(const char *s) {
    if (s[0] == '\'') {
        return (unsigned char)s[1];
    }
    return (int)strtol(s, NULL, 0);
}

/*
 * Parse an operand that could be a #immediate or a label name.
 * If it starts with # the rest is a number. Otherwise it is looked up
 * as a label (pass 2 only, returns 0 if not found in pass 1).
 */
static int parse_imm_or_label(Assembler *as, const char *s, int pass) {
    const char *val = s;
    if (val[0] == '#') val++;  /* strip optional # prefix */

    /* Try as a label first (only meaningful in pass 2) */
    if (pass == 2) {
        int addr = find_label(as, val);
        if (addr >= 0)
            return addr;
    }

    /* Not a label (or pass 1), parse as a number */
    if (val[0] == '-' || val[0] == '+' || isdigit((unsigned char)val[0]) || val[0] == '\'')
        return parse_number(val);

    /* In pass 1 we can't resolve labels yet, return 0 as placeholder */
    if (pass == 1)
        return 0;

    fprintf(stderr, "Error: undefined label '%s'\n", val);
    return 0;
}

/* -----------------------------------------------------------------------
 * Output emission
 * ----------------------------------------------------------------------- */

/*
 * emit -- write one 16-bit instruction word into the output buffer
 *
 * Every time an instruction is encoded in pass 2, emit() stores the result
 * and advances both the output index and the address counter by one word.
 * MOVI and RET call emit() twice because they produce two words each.
 */
static void emit(Assembler *as, uint16_t word) {
    if (as->output_size < MAX_OUTPUT)
        as->output[as->output_size] = word;
    as->output_size++;
    as->current_addr++;
}

/* -----------------------------------------------------------------------
 * Instruction encoding helpers
 * ----------------------------------------------------------------------- */

/* Standard register instruction: [opcode:4][dst:3][src:3][000000] */
static uint16_t encode_reg(uint8_t opcode, uint8_t dst, uint8_t src) {
    return (uint16_t)((opcode << 12) | (dst << 9) | (src << 6));
}

/* -----------------------------------------------------------------------
 * Line processing (called once per line, per pass)
 * ----------------------------------------------------------------------- */

/*
 * Split a line into up to 'max' whitespace/comma-separated tokens.
 * Strips comments first. Returns the number of tokens found.
 */
static int tokenize(char *line, char *tokens[], int max) {
    /* Strip comment */
    char *semi = strchr(line, ';');
    if (semi) *semi = '\0';

    int n = 0;
    char *p = line;
    while (n < max) {
        while (*p && (isspace((unsigned char)*p) || *p == ',')) p++;
        if (!*p) break;

        /* Handle character literal 'X' as one token */
        if (*p == '\'') {
            tokens[n++] = p;
            p++;                  /* skip opening quote */
            if (*p) p++;          /* skip the character */
            if (*p == '\'') p++;  /* skip closing quote */
            /* null-terminate only if followed by separator */
            if (*p && (*p == ',' || isspace((unsigned char)*p))) *p++ = '\0';
            continue;
        }

        tokens[n++] = p;
        while (*p && !isspace((unsigned char)*p) && *p != ',') p++;
        if (*p) *p++ = '\0';
    }
    return n;
}

/*
 * Strip brackets from "[R3]" -> return register number, or -1 on error.
 */
static int parse_bracket_reg(const char *s) {
    if (s[0] != '[') return -1;
    char buf[8];
    int len = strlen(s);
    if (len < 4 || s[len - 1] != ']') return -1;
    strncpy(buf, s + 1, len - 2);
    buf[len - 2] = '\0';
    return parse_register(buf);
}

/*
 * process_line -- handle one line of assembly source
 *
 * Called once per line in each pass. In pass 1 it only counts addresses
 * and records labels; it never writes output. In pass 2 it encodes each
 * instruction into a 16-bit word and calls emit() to store it.
 *
 * The function handles labels, directives (.word), pseudo-instructions
 * (HALT, RET, SUBI), and all 16 real instructions of the EduCore16 ISA.
 */
static void process_line(Assembler *as, char *raw, int pass, int line_num) {
    char line[MAX_LINE_LEN];
    strncpy(line, raw, MAX_LINE_LEN - 1);
    line[MAX_LINE_LEN - 1] = '\0';

    char *tokens[16];
    int ntok = tokenize(line, tokens, 16);
    if (ntok == 0) return;

    int ti = 0;  /* token index */

    /* Check for label (token ending with ':') */
    int len = strlen(tokens[0]);
    if (len > 1 && tokens[0][len - 1] == ':') {
        tokens[0][len - 1] = '\0';
        if (pass == 1)
            add_label(as, tokens[0], as->current_addr);
        ti = 1;
        if (ti >= ntok) return;
    }

    char *mnem = tokens[ti];

    /* ----- Directives ----- */

    if (mnem[0] == '.') {
        if (streqi(mnem, ".word")) {
            for (int i = ti + 1; i < ntok; i++) {
                if (pass == 2) {
                    int val;
                    int lbl = find_label(as, tokens[i]);
                    if (lbl >= 0)
                        val = lbl;
                    else
                        val = parse_number(tokens[i]);
                    emit(as, (uint16_t)val);
                } else {
                    as->current_addr++;
                }
            }
            return;
        }
        fprintf(stderr, "Warning: unknown directive '%s' on line %d\n", mnem, line_num);
        return;
    }

    /* ----- Pseudo-instructions ----- */

    if (streqi(mnem, "HALT")) {
        /* HALT = JMP with offset 0 (jumps to itself) */
        if (pass == 2)
            emit(as, encode_reg(OP_JMP, 0, 0));  /* 0xB000: bit11=0, offset=0 */
        else
            as->current_addr++;
        return;
    }

    if (streqi(mnem, "RET")) {
        /* RET = POP R7; JMP R7 (register-indirect) */
        if (pass == 2) {
            emit(as, encode_reg(OP_POP, 7, 0));        /* POP R7 */
            emit(as, (uint16_t)((OP_JMP << 12) | (1 << 11) | (7 << 6)));  /* JMP R7 */
        } else {
            as->current_addr += 2;
        }
        return;
    }

    if (streqi(mnem, "SUBI")) {
        /* SUBI Rd, #n -> ADDI Rd, #(-n) */
        if (ti + 2 >= ntok) {
            fprintf(stderr, "Error: SUBI needs register and immediate on line %d\n", line_num);
            return;
        }
        int rd = parse_register(tokens[ti + 1]);
        int val = parse_imm_or_label(as, tokens[ti + 2], pass);
        if (rd < 0) {
            fprintf(stderr, "Error: bad register '%s' on line %d\n", tokens[ti + 1], line_num);
            return;
        }
        if (pass == 2) {
            int neg = (-val) & 0x3F;  /* 6-bit two's complement */
            emit(as, (uint16_t)((OP_ADDI << 12) | (rd << 9) | neg));
        } else {
            as->current_addr++;
        }
        return;
    }

    /* ----- Real instructions ----- */

    /* MOV Rd, Rs */
    if (streqi(mnem, "MOV")) {
        int rd = parse_register(tokens[ti + 1]);
        int rs = parse_register(tokens[ti + 2]);
        if (pass == 2) emit(as, encode_reg(OP_MOV, rd, rs));
        else as->current_addr++;
        return;
    }

    /* MOVI Rd, #imm16  or  MOVI Rd, label */
    if (streqi(mnem, "MOVI")) {
        int rd = parse_register(tokens[ti + 1]);
        int val = parse_imm_or_label(as, tokens[ti + 2], pass);
        if (pass == 2) {
            emit(as, (uint16_t)((OP_MOVI << 12) | (rd << 9)));
            emit(as, (uint16_t)val);
        } else {
            as->current_addr += 2;
        }
        return;
    }

    /* LOAD Rd, [Rs] */
    if (streqi(mnem, "LOAD")) {
        int rd = parse_register(tokens[ti + 1]);
        int rs = parse_bracket_reg(tokens[ti + 2]);
        if (rs < 0) {
            fprintf(stderr, "Error: LOAD needs [Rx] on line %d\n", line_num);
            return;
        }
        if (pass == 2) emit(as, encode_reg(OP_LOAD, rd, rs));
        else as->current_addr++;
        return;
    }

    /* STORE [Rd], Rs */
    if (streqi(mnem, "STORE")) {
        int rd = parse_bracket_reg(tokens[ti + 1]);
        int rs = parse_register(tokens[ti + 2]);
        if (rd < 0) {
            fprintf(stderr, "Error: STORE needs [Rx] on line %d\n", line_num);
            return;
        }
        if (pass == 2) emit(as, encode_reg(OP_STORE, rd, rs));
        else as->current_addr++;
        return;
    }

    /* ADD Rd, Rs */
    if (streqi(mnem, "ADD")) {
        int rd = parse_register(tokens[ti + 1]);
        int rs = parse_register(tokens[ti + 2]);
        if (pass == 2) emit(as, encode_reg(OP_ADD, rd, rs));
        else as->current_addr++;
        return;
    }

    /* ADDI Rd, #imm6 */
    if (streqi(mnem, "ADDI")) {
        int rd = parse_register(tokens[ti + 1]);
        int val = parse_imm_or_label(as, tokens[ti + 2], pass);
        if (val < -32 || val > 31) {
            fprintf(stderr, "Warning: ADDI immediate %d out of 6-bit range on line %d\n",
                    val, line_num);
        }
        if (pass == 2) {
            uint16_t imm6 = (uint16_t)(val & 0x3F);
            emit(as, (uint16_t)((OP_ADDI << 12) | (rd << 9) | imm6));
        } else {
            as->current_addr++;
        }
        return;
    }

    /* SUB Rd, Rs */
    if (streqi(mnem, "SUB")) {
        int rd = parse_register(tokens[ti + 1]);
        int rs = parse_register(tokens[ti + 2]);
        if (pass == 2) emit(as, encode_reg(OP_SUB, rd, rs));
        else as->current_addr++;
        return;
    }

    /* AND Rd, Rs */
    if (streqi(mnem, "AND")) {
        int rd = parse_register(tokens[ti + 1]);
        int rs = parse_register(tokens[ti + 2]);
        if (pass == 2) emit(as, encode_reg(OP_AND, rd, rs));
        else as->current_addr++;
        return;
    }

    /* OR Rd, Rs */
    if (streqi(mnem, "OR")) {
        int rd = parse_register(tokens[ti + 1]);
        int rs = parse_register(tokens[ti + 2]);
        if (pass == 2) emit(as, encode_reg(OP_OR, rd, rs));
        else as->current_addr++;
        return;
    }

    /* XOR Rd, Rs */
    if (streqi(mnem, "XOR")) {
        int rd = parse_register(tokens[ti + 1]);
        int rs = parse_register(tokens[ti + 2]);
        if (pass == 2) emit(as, encode_reg(OP_XOR, rd, rs));
        else as->current_addr++;
        return;
    }

    /* CMP Rd, Rs */
    if (streqi(mnem, "CMP")) {
        int rd = parse_register(tokens[ti + 1]);
        int rs = parse_register(tokens[ti + 2]);
        if (pass == 2) emit(as, encode_reg(OP_CMP, rd, rs));
        else as->current_addr++;
        return;
    }

    /* JMP label  (PC-relative)  or  JMP Rx  (register-indirect) */
    if (streqi(mnem, "JMP")) {
        int rs = parse_register(tokens[ti + 1]);
        if (rs >= 0) {
            /* Register-indirect: bit 11 = 1, src in bits 8:6 */
            if (pass == 2)
                emit(as, (uint16_t)((OP_JMP << 12) | (1 << 11) | (rs << 6)));
            else
                as->current_addr++;
        } else {
            /* PC-relative: bit 11 = 0, offset in bits 10:0 (11-bit signed) */
            if (pass == 2) {
                int target = parse_imm_or_label(as, tokens[ti + 1], pass);
                int offset = target - (int)as->current_addr;
                if (offset < -1024 || offset > 1023)
                    fprintf(stderr, "Warning: JMP offset %d out of 11-bit range on line %d\n",
                            offset, line_num);
                uint16_t off11 = (uint16_t)(offset & 0x7FF);
                emit(as, (uint16_t)((OP_JMP << 12) | off11));
            } else {
                as->current_addr++;
            }
        }
        return;
    }

    /* BEQ label  (PC-relative, 12-bit offset) */
    if (streqi(mnem, "BEQ")) {
        if (pass == 2) {
            int target = parse_imm_or_label(as, tokens[ti + 1], pass);
            int offset = target - (int)as->current_addr;
            if (offset < -2048 || offset > 2047)
                fprintf(stderr, "Warning: BEQ offset %d out of 12-bit range on line %d\n",
                        offset, line_num);
            uint16_t off12 = (uint16_t)(offset & 0xFFF);
            emit(as, (uint16_t)((OP_BEQ << 12) | off12));
        } else {
            as->current_addr++;
        }
        return;
    }

    /* CALL Rs */
    if (streqi(mnem, "CALL")) {
        int rs = parse_register(tokens[ti + 1]);
        if (rs < 0) {
            fprintf(stderr, "Error: CALL needs a register on line %d\n", line_num);
            return;
        }
        if (pass == 2) emit(as, encode_reg(OP_CALL, 0, rs));
        else as->current_addr++;
        return;
    }

    /* PUSH Rs */
    if (streqi(mnem, "PUSH")) {
        int rs = parse_register(tokens[ti + 1]);
        if (pass == 2) emit(as, encode_reg(OP_PUSH, 0, rs));
        else as->current_addr++;
        return;
    }

    /* POP Rd */
    if (streqi(mnem, "POP")) {
        int rd = parse_register(tokens[ti + 1]);
        if (pass == 2) emit(as, encode_reg(OP_POP, rd, 0));
        else as->current_addr++;
        return;
    }

    fprintf(stderr, "Error: unknown instruction '%s' on line %d\n", mnem, line_num);
}

/* -----------------------------------------------------------------------
 * Main assemble function
 * ----------------------------------------------------------------------- */

/*
 * asm_init -- reset the assembler to a clean starting state
 *
 * Must be called before asm_assemble(). Zeroes all fields so there are
 * no leftover labels or output words from a previous run.
 */
void asm_init(Assembler *as) {
    memset(as, 0, sizeof(Assembler));
}

/*
 * asm_assemble -- run both passes and write the binary output file
 *
 * Opens src_path, runs pass 1 to collect labels, then runs pass 2 to
 * encode all instructions. Finally writes the output buffer to bin_path
 * in big-endian byte order (high byte first) so memory_load_file() in
 * the emulator can read it directly.
 *
 * Returns 0 on success, -1 if any file could not be opened.
 * Error messages are printed to stderr.
 */
int asm_assemble(Assembler *as, const char *src_path, const char *bin_path) {
    FILE *f;
    char line[MAX_LINE_LEN];
    int line_num;

    /* --- Pass 1: collect labels and count addresses --- */

    f = fopen(src_path, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open '%s'\n", src_path);
        return -1;
    }

    printf("Pass 1: collecting labels...\n");
    as->current_addr = 0;
    line_num = 0;
    while (fgets(line, sizeof(line), f)) {
        line_num++;
        process_line(as, line, 1, line_num);
    }
    fclose(f);

    printf("  %d label(s) found\n", as->label_count);
    for (int i = 0; i < as->label_count; i++)
        printf("    %-16s = 0x%04X\n", as->labels[i].name, as->labels[i].address);

    /* --- Pass 2: generate binary --- */

    f = fopen(src_path, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open '%s'\n", src_path);
        return -1;
    }

    printf("Pass 2: generating code...\n");
    as->current_addr = 0;
    as->output_size  = 0;
    line_num = 0;
    while (fgets(line, sizeof(line), f)) {
        line_num++;
        process_line(as, line, 2, line_num);
    }
    fclose(f);

    printf("  %d word(s) generated\n", as->output_size);

    /* --- Write binary output (big-endian, matching memory_load_file) --- */

    FILE *out = fopen(bin_path, "wb");
    if (!out) {
        fprintf(stderr, "Error: cannot create '%s'\n", bin_path);
        return -1;
    }

    for (int i = 0; i < as->output_size; i++) {
        uint8_t hi = (as->output[i] >> 8) & 0xFF;
        uint8_t lo =  as->output[i]       & 0xFF;
        fputc(hi, out);
        fputc(lo, out);
    }

    fclose(out);
    printf("Wrote %d words to '%s'\n", as->output_size, bin_path);
    return 0;
}
