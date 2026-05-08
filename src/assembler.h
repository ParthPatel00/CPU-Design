/*
 * assembler.h -- Two-pass assembler for the 16-bit CPU
 *
 * The assembler reads human-readable assembly source (.asm) and produces
 * a binary file (.bin) that the emulator can load and run.
 *
 * Two passes are required because a forward reference like "JMP done" needs
 * the address of "done:", which may not have been seen yet when the JMP is
 * first encountered.
 *
 *   Pass 1: scan every line, track the current address (accounting for
 *           multi-word instructions), and record each label's address.
 *
 *   Pass 2: scan again, this time encoding each instruction into binary.
 *           Labels are resolved using the table built in pass 1.
 */

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>

#define MAX_LABELS      256
#define MAX_LINE_LEN    256
#define MAX_OUTPUT    65536

/* One entry in the symbol table: a label name and its word address. */
typedef struct {
    char     name[64];
    uint16_t address;
} Label;

/* Complete assembler state carried across both passes. */
typedef struct {
    Label    labels[MAX_LABELS];
    int      label_count;

    uint16_t output[MAX_OUTPUT];
    int      output_size;

    uint16_t current_addr;
} Assembler;

/* Reset all assembler state to zero. */
void asm_init(Assembler *as);

/*
 * Assemble a source file into a binary file.
 * Returns 0 on success, -1 on error.
 */
int asm_assemble(Assembler *as, const char *src_path, const char *bin_path);

#endif /* ASSEMBLER_H */
