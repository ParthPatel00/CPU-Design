/*
 * io.h -- Memory-mapped I/O handler interface
 *
 * Memory-mapped IO means devices are controlled by reading and writing to
 * specific memory addresses rather than using special IO instructions.
 *
 * When the CPU executes a STORE to an address in the IO region (0xFF00+),
 * memory_write intercepts it and calls io_handle here instead of writing
 * to the memory array. The CPU does not know or care -- it just did a STORE.
 *
 * Current IO devices:
 *   0xFF00  Character output  -- prints the low byte of the stored value as ASCII
 *   0xFF01  Number output     -- prints the stored value as an unsigned decimal
 *   0xFF02  Timer             -- displays the value as a timer countdown reading
 */

#ifndef IO_H
#define IO_H

#include <stdint.h>

/*
 * io_handle -- respond to a write to a memory-mapped IO address
 *
 * Called by memory_write when addr >= IO_BASE (0xFF00).
 * Each address maps to a different device or action.
 * Unknown addresses are silently ignored.
 */
void io_handle(uint16_t addr, uint16_t data);

#endif /* IO_H */
