/*
 * io.c -- Memory-mapped I/O device implementations
 *
 * Each case in the switch statement below is one "device". In real hardware,
 * each device is a separate chip on the motherboard with its own address range.
 * In our emulator, they are just cases in a function.
 *
 * fflush(stdout) is called after every output so the characters appear
 * immediately rather than being held in the C output buffer.
 */

#include <stdio.h>
#include "io.h"
#include "memory.h"

/*
 * io_handle -- dispatch a write to the correct IO device
 *
 * addr -- the IO address that was written to (>= 0xFF00)
 * data -- the 16-bit value that was stored
 *
 * IO_CHAR_OUT (0xFF00):
 *   Treats the low 8 bits of data as an ASCII character code and prints it.
 *   This is how Hello World outputs each letter: STORE [R1], R2 where R1
 *   holds 0xFF00 and R2 holds the character code.
 *
 * IO_NUM_OUT (0xFF01):
 *   Prints the full 16-bit value as an unsigned decimal number.
 *   Used by the Fibonacci program to display each term.
 *
 * IO_TIMER (0xFF02):
 *   Displays the value with a [TIMER] label to show the countdown.
 *   Used by the timer program to show the value on each cycle.
 */
void io_handle(uint16_t addr, uint16_t data) {
    switch (addr) {

        case IO_CHAR_OUT:
            /* Print the low byte as an ASCII character (e.g., 72 = 'H') */
            putchar(data & 0xFF);
            fflush(stdout);
            break;

        case IO_NUM_OUT:
            /* Print the value as an unsigned decimal followed by a space */
            printf("%u ", data);
            fflush(stdout);
            break;

        case IO_TIMER:
            /* Print the value with a label so timer output is easy to follow */
            printf("[TIMER] %u\n", data);
            fflush(stdout);
            break;

        default:
            /* Any write to an unrecognized IO address is silently dropped */
            break;
    }
}
