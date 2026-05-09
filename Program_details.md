# Program Details

This document describes how each assembly program works, how it uses the CPU's instruction set, and how to run it. For the timer program, we walk through the fetch/decode/execute cycle. For the multiply program, we cover the C-to-assembly translation, memory layout, function calls, and recursion with stack frames.

All commands assume you are in the `src/` directory and have already run `make`.

---

## Timer: Fetch/Decode/Execute Cycles

```bash
./asm ../programs/timer.asm ../programs/timer.bin
./cpu verbose ../programs/timer.bin
```

Every instruction goes through three stages: Fetch, Decode, and Execute. Run with `./cpu verbose` to see this for every cycle.

The program starts with two MOVI instructions that load the countdown value into R0 and the timer IO address (0xFF02) into R1. Then it enters a loop: STORE writes R0 to the timer port (printing the value), ADDI decrements R0 by 1, BEQ checks the Zero flag to see if R0 hit 0, and JMP loops back to the STORE if it hasn't. Once ADDI produces 0, Z is set to 1, BEQ branches to HALT, and the CPU stops.

---

## Hello World

```bash
./asm ../programs/hello.asm ../programs/hello.bin
./cpu run ../programs/hello.bin
```

Stores the ASCII string "Hello, World" in memory using a .word directive. The program loops through each character, loading it with LOAD and writing it to the character output IO port at 0xFF00 with STORE. A null terminator (0) marks the end of the string. When the loaded character is 0, BEQ branches to HALT.

Output: `Hello, World`

---

## Fibonacci Sequence

```bash
./asm ../programs/fibonacci.asm ../programs/fibonacci.bin
./cpu run ../programs/fibonacci.bin
```

Computes the first 10 Fibonacci numbers. Two registers hold the current pair (starting with 0 and 1). Each iteration adds them to produce the next value, prints it through the number output IO port at 0xFF01, and stores it in RAM starting at address 0x8000. A counter register tracks how many numbers have been generated. When the counter reaches 10, the program halts.

Output: `0 1 1 2 3 5 8 13 21 34`

---

## Recursive Multiply: Program Layout and Execution

### The C Program

`programs/multiply.c` contains a recursive multiply function: if b is 0, return 0; otherwise, return a plus multiply(a, b minus 1). The main function calls multiply(3, 2) and prints the result.

```bash
gcc -o ../programs/multiply ../programs/multiply.c
../programs/multiply
```

Output: `6`

### C to Assembly Translation

`programs/multiply.asm` translates the C code into assembly. The comments in the file map each section back to the C code.

- **Main**: Load arguments into R0 and R1, load the function address into R6, CALL R6. After the call returns, R0 has the result. Print it via 0xFF01, print a newline via 0xFF00, HALT.
- **Base case check**: ADDI R1, #0 sets the Zero flag without changing b. If b is 0, BEQ jumps to the base case.
- **Saving state**: PUSH R0 and PUSH R1 save a and b on the stack before recursing, since the recursive call overwrites R0 and R1.
- **Recursive call**: ADDI R1, #-1 decrements b, then CALL multiply again.
- **Restoring and combining**: POP R1 and POP R2 restore b and a. ADD R0, R2 adds a to the recursive result.
- **Base case**: MOVI R0, #0 and RET.

```bash
./asm ../programs/multiply.asm ../programs/multiply.bin
./cpu run ../programs/multiply.bin
```

Output: `6`, same as the C version.

### Memory Layout

The binary is loaded starting at address 0x0000. The main program occupies 0x0000 through 0x000F. MOVI instructions take two words (instruction + 16-bit value), single-word instructions like CALL and HALT take one.

The multiply function starts at address 0x0010. The assembler records label addresses in the symbol table during pass 1, so `MOVI R6, #multiply` at 0x0004 resolves to 0x0010.

The base case (mul_base) starts at 0x001D. The entire program is 33 words.

### Function Calls

```bash
./cpu verbose ../programs/multiply.bin
```

Cycles 0 through 2 load arguments: R0 gets 3, R1 gets 2, R6 gets 0x0010. SP is still 0xFEFF.

Cycle 3 is the CALL instruction. The return address (0x0007) gets pushed onto the stack, PC jumps to 0x0010. SP moves from 0xFEFF to 0xFEFE.

### Recursion: Stack Growth and Unwinding

Each recursive call builds a stack frame of 3 words: the return address pushed by CALL, then saved R0 (a), then saved R1 (b).

**First call: multiply(3, 2)**

Cycle 4 tests if b is zero. It's 2, so no branch. Cycles 6 and 7 PUSH R0 and R1, saving a=3 and b=2. SP drops from 0xFEFE to 0xFEFD to 0xFEFC. Cycle 8 decrements b from 2 to 1. Cycle 10 is another CALL, SP drops to 0xFEFB.

**Second call: multiply(3, 1)**

Same pattern. Cycles 13 and 14 push a=3 and b=1. SP drops to 0xFEFA, then 0xFEF9. Cycle 15 decrements b to 0. Cycle 17 is another CALL. SP drops to 0xFEF8. This is the deepest the stack gets: 7 words.

**Third call: multiply(3, 0), the base case**

Cycle 18 tests b. It's 0, Z flag is 1. BEQ branches to mul_base at 0x001D. R0 gets 0 (base case return value). RET pops the return address and jumps back. SP goes from 0xFEF8 back to 0xFEF9.

**Unwinding, second call**

POP restores b=1 and a=3. ADD R0, R2 computes 0 + 3 = 3. RET returns. SP goes to 0xFEFC.

**Unwinding, first call**

POP restores b=2 and a=3. ADD R0, R2 computes 3 + 3 = 6. RET returns. SP goes back to 0xFEFF, exactly where it started. R0 holds 6.

The stack grows 3 words per recursive call, hits the deepest point at the base case, then shrinks 3 words per return. At the end, SP is back to 0xFEFF and R0 holds 6.
