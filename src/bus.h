/*
 * bus.h -- system bus interface
 *
 * In real hardware, the bus is the set of physical wires that connects the CPU
 * to memory and all devices. When the CPU wants to read or write data, it puts
 * the address on the address bus and the data on the data bus, then asserts a
 * read or write signal. Everything else listens and the device at that address
 * responds.
 *
 * In our emulator, the bus is a software abstraction: a pair of functions that
 * the control unit calls for every memory access. The bus then routes the
 * request to the correct destination (RAM or IO), hiding that detail from the
 * control unit entirely.
 *
 * The control unit always talks to the bus. It never calls memory_read or
 * memory_write directly. This mirrors the real hardware separation and makes
 * it easy to add new devices later without touching the control unit.
 */

#ifndef BUS_H
#define BUS_H

#include <stdint.h>

/*
 * bus_read -- read a 16-bit word from the given address
 *
 * Currently routes all reads to memory_read. IO devices in this design are
 * write-only (you write to them to produce output; you never read back from
 * them), so every read address goes to RAM.
 */
uint16_t bus_read(uint16_t addr);

/*
 * bus_write -- write a 16-bit word to the given address
 *
 * Routes the write to memory_write, which intercepts IO addresses (>= 0xFF00)
 * and hands them to io_handle. Addresses below 0xFF00 are written to RAM.
 */
void bus_write(uint16_t addr, uint16_t data);

#endif /* BUS_H */
