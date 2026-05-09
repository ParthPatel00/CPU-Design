/*
 * memory.c -- memory array implementation
 *
 * Provides the backing storage for the CPU's entire 64K address space.
 * All reads and writes from the control unit go through bus.c, which calls
 * memory_read / memory_write here. This file is also responsible for
 * intercepting writes to the IO region (0xFF00+) and routing them to io.c.
 */

#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "io.h"

/*
 * The memory array -- 65,536 16-bit words representing the full address space.
 * Declared as a global so all components can share it without passing it around.
 * memory[0] = address 0x0000, memory[65535] = address 0xFFFF.
 */
uint16_t memory[MEM_SIZE];

/*
 * memory_init -- clear all memory to zero
 *
 * Called once at startup before any program is loaded. Ensures there is no
 * leftover garbage in memory that could cause undefined behavior.
 */
void memory_init(void) {
    memset(memory, 0, sizeof(memory));
}

/*
 * memory_read -- read one 16-bit word from the given address
 *
 * All reads are simple array lookups. There are no read-triggered IO devices
 * in our design, so every address just returns whatever is in the array.
 */
uint16_t memory_read(uint16_t addr) {
    return memory[addr];
}

/*
 * memory_write -- write one 16-bit word to the given address
 *
 * This is where the memory-mapped IO trick happens. Any write to an address
 * in the IO region (0xFF00 and above) is handed off to io_handle() instead
 * of being stored in the array. The CPU never sees this distinction -- it
 * just performs a normal STORE instruction.
 *
 * Addresses below IO_BASE are written directly to the memory array.
 */
void memory_write(uint16_t addr, uint16_t data) {
    if (addr >= IO_BASE) {
        /* This address belongs to a device, not RAM -- route to IO handler */
        io_handle(addr, data);
        return;
    }
    memory[addr] = data;
}

/*
 * memory_dump -- print a range of memory in a readable hex grid
 *
 * Prints 8 words per row. Each row starts with the address of its first word.
 * Column headers show the offset within the row (+0 through +7).
 *
 * Example:
 *   ADDR    +0      +1      +2      +3      +4      +5      +6      +7
 *   0x8000  0x0000  0x0001  0x0001  0x0002  0x0003  0x0005  0x0008  0x000D
 *
 * Used after running the Fibonacci program to verify the sequence in RAM.
 */
void memory_dump(uint16_t from, uint16_t to) {
    int col;
    uint16_t addr;

    printf("\n--- Memory Dump [0x%04X - 0x%04X] ---\n", from, to);

    /* Print column header */
    printf("ADDR    ");
    for (col = 0; col < 8; col++)
        printf("+%-7d", col);
    printf("\n");

    /* Print each word, 8 per row */
    for (addr = from; addr <= to; addr++) {
        /* Start of a new row: print the row's base address */
        if ((addr - from) % 8 == 0)
            printf("0x%04X  ", addr);

        printf("0x%04X  ", memory[addr]);

        /* End of a row or last address: newline */
        if ((addr - from) % 8 == 7 || addr == to)
            printf("\n");
    }
}

/*
 * memory_load_file -- load a binary program file into memory
 *
 * Opens the file at 'path' and reads it two bytes at a time, combining each
 * pair into one 16-bit word stored at sequential addresses starting at 0x0000.
 *
 * Byte order is big-endian: the first byte of each pair is the high byte and
 * the second byte is the low byte. So a file containing 0x10 0x20 produces
 * the word 0x1020 at address 0x0000.
 *
 * This is the format the assembler produces. Returns 0 on success, -1 on error.
 */
int memory_load_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Error: cannot open file '%s'\n", path);
        return -1;
    }

    uint16_t addr = 0;
    int hi, lo;

    /* Read two bytes at a time and combine into one 16-bit word */
    while ((hi = fgetc(f)) != EOF && (lo = fgetc(f)) != EOF) {
        memory[addr++] = ((uint16_t)hi << 8) | (uint16_t)lo;
    }

    fclose(f);
    return 0;
}
