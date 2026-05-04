/*
 * memory.h -- memory array and memory-mapped IO address definitions
 *
 * Our CPU has a 16-bit address space, meaning it can reference 2^16 = 65,536
 * unique locations. Each location holds one 16-bit word (not one byte).
 * This is called word-addressed memory.
 *
 * The address space is divided into three regions:
 *
 *   0x0000 - 0x7FFF   Program space  (32,768 words) -- instructions live here
 *   0x8000 - 0xFEFF   RAM            (28,416 words) -- data, stack, program output
 *   0xFF00 - 0xFFFF   Memory-mapped IO (256 words)  -- writing here talks to devices
 *
 * Memory-mapped IO means there are no special IO instructions. To print a
 * character, the program simply does a STORE to address 0xFF00. The
 * memory_write function intercepts that address and calls io_handle instead
 * of writing to the array. The CPU does not know the difference.
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

/* Total number of 16-bit word locations in the address space */
#define MEM_SIZE     0x10000

/* IO region -- any write to address >= IO_BASE is intercepted */
#define IO_BASE      0xFF00

/* Individual IO device addresses */
#define IO_CHAR_OUT  0xFF00   /* STORE here: low byte is printed as an ASCII character */
#define IO_NUM_OUT   0xFF01   /* STORE here: value is printed as an unsigned decimal   */
#define IO_TIMER     0xFF02   /* STORE here: value is displayed as a timer countdown   */

/* The memory array. Declared extern so all files share the same instance. */
extern uint16_t memory[MEM_SIZE];

/* Zero out the entire memory array before loading a program. */
void     memory_init(void);

/* Return the 16-bit word stored at the given address. */
uint16_t memory_read(uint16_t addr);

/*
 * Write a 16-bit word to the given address.
 * If addr >= IO_BASE, the write is routed to io_handle instead of the array.
 */
void     memory_write(uint16_t addr, uint16_t data);

/*
 * Print memory contents from address 'from' to 'to' inclusive.
 * Output is formatted as a grid of 8 words per row with hex addresses and values.
 * Used after running Fibonacci to inspect the stored sequence in RAM.
 */
void     memory_dump(uint16_t from, uint16_t to);

/*
 * Load a binary file into memory starting at address 0x0000.
 * The file is produced by the assembler. Each pair of bytes becomes one
 * 16-bit word (big-endian: high byte first, low byte second).
 * Returns 0 on success, -1 if the file cannot be opened.
 */
int      memory_load_file(const char *path);

#endif /* MEMORY_H */
