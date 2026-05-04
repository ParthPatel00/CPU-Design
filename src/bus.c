/*
 * bus.c -- system bus implementation
 *
 * The bus is intentionally thin. Its only job is to be the single point of
 * contact between the control unit and the rest of the system. All reads and
 * writes from the control unit pass through here before reaching memory or IO.
 *
 * Why not call memory_read/memory_write directly from the control unit?
 * Because in real hardware the CPU never touches memory directly -- it puts
 * signals on the bus and the bus connects everything. Keeping this layer
 * makes the emulator structure match the real hardware architecture.
 */

#include "bus.h"
#include "memory.h"

/*
 * bus_read -- forward a read request to memory
 *
 * The control unit calls this every time it needs to fetch an instruction
 * (during the FETCH stage) or load data (LOAD instruction, POP instruction,
 * or fetching the second word of a MOVI instruction).
 */
uint16_t bus_read(uint16_t addr) {
    return memory_read(addr);
}

/*
 * bus_write -- forward a write request to memory
 *
 * The control unit calls this for STORE, PUSH, and CALL (which saves the
 * return address to the stack). memory_write will intercept IO addresses
 * and route them to the appropriate device.
 */
void bus_write(uint16_t addr, uint16_t data) {
    memory_write(addr, data);
}
