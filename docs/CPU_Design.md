# Software CPU Design

## How to Build and Run

```bash
cd src
make              # builds both 'cpu' (emulator) and 'asm' (assembler)

# Assemble a program
./asm ../programs/hello.asm ../programs/hello.bin

# Run it
./cpu run ../programs/hello.bin

# Run with cycle-by-cycle trace (Fetch/Decode/Execute output)
./cpu verbose ../programs/timer.bin

# Run then dump a memory range
./cpu dump ../programs/fibonacci.bin 0x8000 0x8009
```

---

## CPU Schematic

```
  +------------------------------------------------------------------+
  |                           CPU CHIP                                |
  |                                                                   |
  |   +--------------------+                                          |
  |   |   Control Unit     |    Runs the Fetch / Decode / Execute     |
  |   |   +-----+  +----+ |    cycle. Reads instruction from memory, |
  |   |   | PC  |  | IR | |    decodes fields, tells other           |
  |   |   +--+--+  +--+-+ |    components what to do.                |
  |   +------|--------+---+                                           |
  |          |        |                                               |
  |   +------v--------v-------+                                      |
  |   |     Internal Bus      |                                      |
  |   +--+---------+--------+-+                                      |
  |      |         |        |                                        |
  |  +---v----+ +--v----+ +-v-----------+                            |
  |  |  ALU   | |  Reg  | |   FLAGS     |                            |
  |  | ADD    | |  File | |  Z N C V    |                            |
  |  | SUB    | | R0-R7 | +-------------+                            |
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

## Emulator

### Source Files

| File                  | Component        | Purpose                                          |
| --------------------- | ---------------- | ------------------------------------------------ |
| cpu.h / cpu.c         | Registers        | CPU struct (R0-R7, PC, SP, FLAGS), init, print   |
| alu.h / alu.c         | ALU              | ADD, SUB, AND, OR, XOR, flag computation         |
| control.h / control.c | Control Unit     | Fetch/decode/execute loop, halt detection        |
| bus.h / bus.c         | Bus              | Routes reads/writes between CPU and memory       |
| memory.h / memory.c   | Memory           | 64K word array, IO interception, dump, file load |
| io.h / io.c           | Memory-mapped IO | Character, number, and timer output              |
| main.c                | Entry point      | Parses args, runs in run/verbose/dump mode       |

### Fetch / Decode / Execute Cycle

Every instruction goes through three stages:

1. **Fetch**: Read `mem[PC]` into IR, save instruction address, increment PC. MOVI fetches a second word.
2. **Decode**: Extract opcode (bits 15:12), dst (11:9), src (8:6), immediate/offset fields.
3. **Execute**: Perform the operation (ALU, memory access, or PC update). Update FLAGS if applicable.

The loop repeats until HALT is detected (JMP with offset 0, causing PC to point at itself).

---

## Assembler

### Source Files

| File                      | Purpose                                       |
| ------------------------- | --------------------------------------------- |
| assembler.h / assembler.c | Two-pass assembler implementation             |
| asm_main.c                | Entry point: `./asm <input.asm> <output.bin>` |

### Design

Two-pass assembly:

- **Pass 1**: Scan all lines, track current address, record each `label:` and its address in a symbol table.
- **Pass 2**: Scan again, encode each instruction to binary, resolve label references from the symbol table.

Output is big-endian 16-bit words matching the emulator's binary loader.

### Supported Syntax

- **Labels**: `loop:` at start of line, referenced by JMP/BEQ/MOVI
- **Registers**: R0-R7 (case-insensitive)
- **Immediates**: `#10`, `#-3`, `#0xFF00`
- **Character literals**: `'A'` (in .word directives)
- **Comments**: `; rest of line`
- **Directives**: `.word v1, v2, ...` emits raw 16-bit values

---

## Programs

### Timer (programs/timer.asm)

Counts down from 100 to 1, writing each value to the timer IO register. Run with `./cpu verbose timer.bin` to see the Fetch/Decode/Execute cycle step by step.

```
$ ./cpu run programs/timer.bin
[TIMER] 100
[TIMER] 99
...
[TIMER] 1
```

### Hello, World (programs/hello.asm)

Prints "Hello, World" one character at a time through the character output IO register.

```
$ ./cpu run programs/hello.bin
Hello, World
```

### Fibonacci Sequence (programs/fibonacci.asm)

Computes and prints the first 10 Fibonacci numbers, and stores them in RAM at 0x8000-0x8009.

```
$ ./cpu run programs/fibonacci.bin
0 1 1 2 3 5 8 13 21 34

$ ./cpu dump programs/fibonacci.bin 0x8000 0x8009
ADDR    +0      +1      +2      +3      +4      +5      +6      +7
0x8000  0x0000  0x0001  0x0001  0x0002  0x0003  0x0005  0x0008  0x000D
0x8008  0x0015  0x0022
```
