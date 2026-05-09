# Software CPU Design

CMPE 220, System Software | San Jose State University | Spring 2026

Group 6: Parth Patel & Harshitha Vadavalli

GitHub: https://github.com/ParthPatel00/CPU-Design

## Team Contributions

| Team Member             | Work                                                                                                                                                                                              |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Parth Patel**         | CPU architecture design, ISA, instruction encoding, memory map. Implemented emulator modules (cpu.c, alu.c, control.c, bus.c, memory.c, io.c, main.c). Wrote ISA documentation and CPU schematic. |
| **Harshitha Vadavalli** | Two-pass assembler (assembler.c, asm_main.c). Assembly programs for Timer, Hello world, Fibonnaci, and Multiply. Makefile targets. Project reports.                                               |

## How to Build and Run

```bash
cd src
make              # builds both 'cpu' (emulator) and 'asm' (assembler)
```

If you see `make: Nothing to be done for 'all'`, it's already compiled. Run `make clean && make` to rebuild.

Assemble a program, then run it:

```bash
./asm ../programs/hello.asm ../programs/hello.bin
./cpu run ../programs/hello.bin
```

`./cpu run` executes silently, only showing program output:

```bash
./asm ../programs/hello.asm ../programs/hello.bin
./cpu run ../programs/hello.bin          # Hello, World

./asm ../programs/fibonacci.asm ../programs/fibonacci.bin
./cpu run ../programs/fibonacci.bin      # 0 1 1 2 3 5 8 13 21 34

./asm ../programs/timer.asm ../programs/timer.bin
./cpu run ../programs/timer.bin          # [TIMER] 3 ... [TIMER] 1

./asm ../programs/multiply.asm ../programs/multiply.bin
./cpu run ../programs/multiply.bin       # 6 (recursive 3 * 2)
```

`./cpu verbose` shows the fetch/decode/execute cycle for every instruction (run one at a time):

```bash
./cpu verbose ../programs/hello.bin
./cpu verbose ../programs/fibonacci.bin
./cpu verbose ../programs/timer.bin
./cpu verbose ../programs/multiply.bin
```

---

## CPU Schematic

```
  +------------------------------------------------------------------+
  |                           CPU CHIP                                |
  |                                                                   |
  |   +--------------------+                                          |
  |   |   Control Unit     |    Runs the Fetch / Decode / Execute     |
  |   |   +-----+  +----+ |    cycle. Reads instruction from memory,  |
  |   |   | PC  |  | IR | |    decodes fields, tells other            |
  |   |   +--+--+  +--+-+ |    components what to do.                 |
  |   +------|--------+---+                                           |
  |          |        |                                               |
  |   +------v--------v-------+                                       |
  |   |     Internal Bus      |                                       |
  |   +--+---------+--------+-+                                       |
  |      |         |        |                                         |
  |  +---v----+ +--v----+ +-v-----------+                             |
  |  |  ALU   | |  Reg  | |   FLAGS     |                             |
  |  | ADD    | |  File | |  Z N C V    |                             |
  |  | SUB    | | R0-R7 | +-------------+                             |
  |  | AND    | | SP    |                                             |
  |  | OR     | +-------+                                             |
  |  | XOR    |                                                       |
  |  +--------+                                                       |
  +-----------------------------+-------------------------------------+
                                |
                     +----------v-----------+
                     |     System Bus       |
                     |  (16-bit addr/data)  |
                     +----------+-----------+
                                |
              +-----------------+-----------------+
              |                                   |
     +--------v---------+             +-----------v-----------+
     |     Memory       |             |   Memory-Mapped IO    |
     |   64K words      |             |   0xFF00 - 0xFFFF     |
     |                  |             |                       |
     | 0x0000: program  |             |  0xFF00: char output  |
     | 0x8000: RAM      |             |  0xFF01: number output|
     +---------+--------+             |  0xFF02: timer display|
               |                      +-----------------------+
               v
     Stack grows downward
     from SP = 0xFEFF
```

---

## ISA Specification

### Registers

| Name    | Width  | Purpose                                                 |
| ------- | ------ | ------------------------------------------------------- |
| R0 - R7 | 16-bit | General purpose (3-bit index)                           |
| PC      | 16-bit | Program Counter                                         |
| SP      | 16-bit | Stack Pointer (starts at 0xFEFF, grows downward)        |
| FLAGS   | 16-bit | Status bits: Z (bit 0), N (bit 1), C (bit 2), V (bit 3) |

### Instruction Set

| Opcode | Binary | Mnemonic | Format     | Operation                               |
| ------ | ------ | -------- | ---------- | --------------------------------------- |
| 0      | 0000   | MOV      | dst, src   | dst = src                               |
| 1      | 0001   | MOVI     | dst, imm16 | dst = 16-bit immediate (2-word)         |
| 2      | 0010   | LOAD     | dst, [src] | dst = mem[src]                          |
| 3      | 0011   | STORE    | [dst], src | mem[dst] = src                          |
| 4      | 0100   | ADD      | dst, src   | dst = dst + src                         |
| 5      | 0101   | ADDI     | dst, imm6  | dst = dst + signed immediate (-32..+31) |
| 6      | 0110   | SUB      | dst, src   | dst = dst - src                         |
| 7      | 0111   | AND      | dst, src   | dst = dst & src                         |
| 8      | 1000   | OR       | dst, src   | dst = dst \| src                        |
| 9      | 1001   | XOR      | dst, src   | dst = dst ^ src                         |
| 10     | 1010   | CMP      | dst, src   | set FLAGS on (dst - src), no writeback  |
| 11     | 1011   | JMP      | label / Rx | PC-relative or register-indirect        |
| 12     | 1100   | BEQ      | label      | jump if Z = 1, PC-relative              |
| 13     | 1101   | CALL     | src        | push PC, jump to src register           |
| 14     | 1110   | PUSH     | src        | mem[--SP] = src                         |
| 15     | 1111   | POP      | dst        | dst = mem[SP++]                         |

### Pseudo-instructions

| Mnemonic     | Assembles to   | Purpose                |
| ------------ | -------------- | ---------------------- |
| HALT         | JMP 0          | Infinite loop = stop   |
| SUBI dst, #n | ADDI dst, #-n  | Subtract immediate     |
| RET          | POP R7; JMP R7 | Return from subroutine |

### Instruction Encoding

**Layout 1 -- Register instructions** (MOV, LOAD, STORE, ADD, SUB, AND, OR, XOR, CMP, CALL, PUSH, POP):

```
 15  14  13  12 | 11  10   9 |  8   7   6 |  5   4   3   2   1   0
[    OPCODE   ] [    DST   ] [    SRC   ] [      000000 (unused)  ]
```

**Layout 2 -- ADDI** (6-bit signed immediate):

```
 15  14  13  12 | 11  10   9 |  8   7   6 |  5   4   3   2   1   0
[    0101     ] [    DST   ] [   000     ] [ signed imm (6 bits) ]
```

**Layout 3 -- MOVI** (two words):

```
Word 1:  [0001][dst:3][000000000]
Word 2:  [full 16-bit value]
```

**Layout 4 -- JMP and BEQ** (PC-relative offsets):

```
JMP (nearby):  [1011][0][signed offset: 11 bits]       range: -1024..+1023
JMP (far):     [1011][1][00][src:3][000000]             jumps to address in register
BEQ:           [1100][signed offset: 12 bits]           range: -2048..+2047
```

### Flag Semantics

| Flag | Bit | Set when                                     |
| ---- | --- | -------------------------------------------- |
| Z    | 0   | Result is exactly 0                          |
| N    | 1   | Result bit 15 is 1 (negative in signed math) |
| C    | 2   | Unsigned carry out of bit 15                 |
| V    | 3   | Signed overflow                              |

Instructions that update FLAGS: ADD, ADDI, SUB, AND, OR, XOR, MOV, LOAD, CMP.
Instructions that do NOT update FLAGS: MOVI, STORE, JMP, BEQ, CALL, PUSH, POP.

### Addressing Modes

| Mode              | Syntax   | Example          |
| ----------------- | -------- | ---------------- |
| Register          | dst, src | ADD R1, R2       |
| Immediate (small) | dst, #n  | ADDI R0, #-1     |
| Immediate (full)  | dst, #n  | MOVI R0, #0x8000 |
| Register-Indirect | [reg]    | LOAD R1, [R2]    |
| PC-Relative       | label    | JMP loop         |

### Memory Map

```
0x0000 - 0x7FFF    Program space (32,768 words)
0x8000 - 0xFEFF    RAM (32,511 words, includes stack)
0xFF00 - 0xFFFF    Memory-mapped IO (256 words)
```

Word-addressed: each address holds one 16-bit word.

| IO Address | Device           | Behavior                           |
| ---------- | ---------------- | ---------------------------------- |
| 0xFF00     | Character output | STORE prints the low byte as ASCII |
| 0xFF01     | Number output    | STORE prints the value as decimal  |
| 0xFF02     | Timer register   | STORE displays as timer countdown  |

---

## Source Files

### Emulator

| File                  | Component        | Purpose                                        |
| --------------------- | ---------------- | ---------------------------------------------- |
| cpu.h / cpu.c         | Registers        | CPU struct (R0-R7, PC, SP, FLAGS), init, print |
| alu.h / alu.c         | ALU              | ADD, SUB, AND, OR, XOR, flag computation       |
| control.h / control.c | Control Unit     | Fetch/decode/execute loop, halt detection      |
| bus.h / bus.c         | Bus              | Routes reads/writes between CPU and memory     |
| memory.h / memory.c   | Memory           | 64K word array, IO interception, file load     |
| io.h / io.c           | Memory-mapped IO | Character, number, and timer output            |
| main.c                | Entry point      | Parses args, runs in run/verbose mode          |

### Assembler

| File                      | Purpose                                       |
| ------------------------- | --------------------------------------------- |
| assembler.h / assembler.c | Two-pass assembler implementation             |
| asm_main.c                | Entry point: `./asm <input.asm> <output.bin>` |

The assembler works in two passes. In the first pass, it scans through the entire source file and records the memory address of every label into a symbol table. In the second pass, it goes through the file again and encodes each instruction into binary. It needs two passes because a jump instruction might reference a label that appears later in the file, so all label addresses must be known before encoding.

### Programs

| File          | Description                                           |
| ------------- | ----------------------------------------------------- |
| hello.asm     | Prints "Hello, World" through character IO            |
| fibonacci.asm | First 10 Fibonacci numbers, printed and stored in RAM |
| timer.asm     | Counts down from 3 to 1 on the timer display          |
| multiply.asm  | Recursive multiply(3, 2) = 6 using CALL/PUSH/POP/RET  |

For detailed execution traces, fetch/decode/execute cycle breakdowns, memory layout, and recursion stack analysis, see [Program_details.md](Program_details.md).

