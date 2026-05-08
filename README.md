# EduCore16 — 16-bit Software CPU in C

**CMPE 220 — System Software | San Jose State University | Spring 2026**

**Team:** Harshitha Vadavalli &amp; Parth Patel

**GitHub:** https://github.com/ParthPatel00/CPU-Design

---

## What This Project Is

EduCore16 is a complete **16-bit CPU emulator and assembler** written in C from scratch.

It simulates every hardware component of a real processor — register file, ALU, control unit, system bus, 64K word memory, and memory-mapped I/O — all separated into dedicated source files that mirror physical CPU architecture. Alongside the emulator, we built a full **two-pass assembler** that compiles `.asm` source files into binary programs the CPU runs.

Five assembly programs demonstrate the full capability of the CPU: Hello World, Fibonacci Sequence, Timer Countdown, Recursive Multiply, and Prime Numbers.

---

## Requirements

- macOS or Linux
- GCC (any modern version)
- Git
- Make

No external libraries. Pure standard C.

---

## Build and Run — 4 Steps

### Step 1 — Clone the Repository

```bash
git clone https://github.com/ParthPatel00/CPU-Design.git
cd CPU-Design/src
```

---

### Step 2 — Build the Emulator and Assembler

```bash
make
```

This compiles all source files and produces two executables in `src/`:

- **`cpu`** — the CPU emulator (load, run, verbose trace, memory dump)
- **`asm`** — the assembler (compiles `.asm` source to `.bin` binary)

---

### Step 3 — Assemble All Programs

```bash
make programs
```

This runs the assembler on all five programs and places the `.bin` files in `programs/`:

```
[ASM] hello.asm
[ASM] fibonacci.asm
[ASM] timer.asm
[ASM] multiply.asm
[ASM] primes.asm
```

---

### Step 4 — Run the Programs

**Hello, World**
```bash
./cpu run ../programs/hello.bin
```
```
Hello, World
```

**Fibonacci Sequence** — first 10 numbers
```bash
./cpu run ../programs/fibonacci.bin
```
```
0 1 1 2 3 5 8 13 21 34
```

**Timer Countdown** — 100 down to 1
```bash
./cpu run ../programs/timer.bin
```
```
[TIMER] 100
[TIMER] 99
...
[TIMER] 1
```

**Recursive Multiply** — multiply(6, 7) = 42 using CALL/PUSH/POP/RET
```bash
./cpu run ../programs/multiply.bin
```
```
42
```

**Prime Numbers** — first 10 primes via trial division
```bash
./cpu run ../programs/primes.bin
```
```
2 3 5 7 11 13 17 19 23 29
```

---

## Fetch / Decode / Execute — Verbose Mode

Watch every instruction cycle step by step:

```bash
./cpu verbose ../programs/timer.bin
```

```
========= Cycle 2 =========
[FETCH]   PC=0x0004  instruction=0x3200
[DECODE]  opcode=0x3  dst=R1  src=R0
[TIMER] 100
[EXECUTE] --- CPU State ---
  R0 = 0x0064 (100)    R1 = 0xFF02 (65282)
  PC = 0x0005          SP = 0xFEFF
  FLAGS = Z:0 N:0 C:0 V:0
```

Every cycle prints all three stages: FETCH reads the raw instruction word, DECODE extracts opcode and register fields, EXECUTE shows the complete CPU state after the instruction ran.

---

## Memory Dump — Inspect RAM Contents

After running Fibonacci, verify what was stored in RAM:

```bash
./cpu dump ../programs/fibonacci.bin 0x8000 0x8009
```

```
--- Memory Dump [0x8000 - 0x8009] ---
ADDR    +0      +1      +2      +3      +4      +5      +6      +7
0x8000  0x0000  0x0001  0x0001  0x0002  0x0003  0x0005  0x0008  0x000D
0x8008  0x0015  0x0022
```

The 10 Fibonacci numbers (0, 1, 1, 2, 3, 5, 8, 13, 21, 34) stored correctly in memory at addresses 0x8000–0x8009.

---

## Automated Test Suite

Run all 7 tests at once — each prints PASS or FAIL:

```bash
make test
```

```
========================================
  EduCore16 Automated Test Suite
========================================

  [TEST] hello        ... PASS
  [TEST] fibonacci    ... PASS
  [TEST] timer (100)  ... PASS
  [TEST] timer (last) ... PASS
  [TEST] multiply     ... PASS
  [TEST] fib dump     ... PASS
  [TEST] primes       ... PASS

  Results: 7 passed, 0 failed
========================================
```

---

## CPU Architecture

```
┌──────────────────────────────────────────────────────┐
│                    EduCore16 CPU                      │
│                                                       │
│  Registers: R0–R7 (general), PC, SP, FLAGS           │
│                                                       │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────┐ │
│  │ Control Unit│  │     ALU      │  │Register File│ │
│  │  F/D/E loop │  │ ADD  SUB     │  │ R0  R1  R2  │ │
│  │  16 opcodes │  │ AND  OR  XOR │  │ R3  R4  R5  │ │
│  │             │  │ CMP  flags   │  │ R6  R7      │ │
│  └──────┬──────┘  └──────────────┘  │ PC  SP FLAGS│ │
│         │                           └─────────────┘ │
└─────────┼──────────────────────────────────────────-─┘
          │ System Bus  (16-bit address / 16-bit data)
          │
   ┌──────┴──────────────────────┐
   │                             │
┌──▼──────────┐    ┌─────────────▼──────────┐
│   Memory    │    │   Memory-Mapped I/O    │
│  64K words  │    │  0xFF00 → char out     │
│ 0x0000–     │    │  0xFF01 → number out   │
│ 0xFEFF      │    │  0xFF02 → timer out    │
└─────────────┘    └────────────────────────┘
```

### Memory Map

| Address Range | Purpose |
|---|---|
| `0x0000 – 0x7FFF` | Program space — code is loaded and executed here |
| `0x8000 – 0xFEFF` | RAM + Stack — data storage; stack grows downward from 0xFEFF |
| `0xFF00 – 0xFFFF` | Memory-Mapped I/O — writes go to output devices, not RAM |

### Registers

| Register | Width | Purpose |
|---|---|---|
| R0 – R7 | 16-bit | General purpose |
| PC | 16-bit | Program Counter — address of next instruction |
| SP | 16-bit | Stack Pointer — starts at 0xFEFF, grows downward |
| FLAGS | 16-bit | Z (zero), N (negative), C (carry), V (overflow) |

---

## Instruction Set — 16 Instructions

| Opcode | Instruction | Operation |
|---|---|---|
| `0x0` | `MOV`   | Copy register to register |
| `0x1` | `MOVI`  | Load 16-bit constant into register (2-word instruction) |
| `0x2` | `LOAD`  | Read memory into register |
| `0x3` | `STORE` | Write register to memory or I/O device |
| `0x4` | `ADD`   | Add two registers |
| `0x5` | `ADDI`  | Add signed 6-bit immediate (−32 to +31) to register |
| `0x6` | `SUB`   | Subtract two registers |
| `0x7` | `AND`   | Bitwise AND |
| `0x8` | `OR`    | Bitwise OR |
| `0x9` | `XOR`   | Bitwise XOR |
| `0xA` | `CMP`   | Compare — sets FLAGS, no writeback |
| `0xB` | `JMP`   | Unconditional jump (PC-relative or register-indirect) |
| `0xC` | `BEQ`   | Branch if Z flag = 1 |
| `0xD` | `CALL`  | Call function — pushes return address, jumps to register |
| `0xE` | `PUSH`  | Push register onto stack |
| `0xF` | `POP`   | Pop from stack into register |

**Pseudo-instructions** (the assembler expands these automatically):

| Write This | Assembler Produces | Purpose |
|---|---|---|
| `HALT` | `JMP 0` (self-loop) | Stops the CPU cleanly |
| `SUBI Rd, #n` | `ADDI Rd, #(-n)` | Subtract a constant from a register |
| `RET` | `POP R7` + `JMP R7` | Return from a subroutine |

---

## Project File Structure

```
CPU-Design/
├── src/
│   ├── Makefile           Build system — make / make programs / make test / make clean
│   ├── cpu.h / cpu.c      Register file (R0–R7, PC, SP, FLAGS), initialization
│   ├── alu.h / alu.c      ALU: ADD, SUB, AND, OR, XOR, CMP — all math and logic with flags
│   ├── control.h / .c     Fetch / Decode / Execute loop — the CPU brain, all 16 opcodes
│   ├── bus.h / bus.c      Routes all reads/writes between CPU, memory, and I/O
│   ├── memory.h / .c      64K word array, big-endian binary loader, hex dump
│   ├── io.h / io.c        Three memory-mapped output devices: character, number, timer
│   ├── assembler.h / .c   Two-pass assembler: .asm source text → .bin binary
│   ├── asm_main.c         Assembler command-line entry point
│   └── main.c             Emulator entry point (run / verbose / dump modes)
├── programs/
│   ├── hello.asm/.bin     Prints "Hello, World" via memory-mapped character I/O
│   ├── fibonacci.asm/.bin First 10 Fibonacci numbers — printed and stored in RAM
│   ├── timer.asm/.bin     Countdown 100→1 on the timer device
│   ├── multiply.asm/.bin  Recursive multiply(6,7)=42 using CALL/PUSH/POP/RET
│   └── primes.asm/.bin    First 10 prime numbers via trial division
└── docs/
    ├── CPU_Design.md                   Full ISA, encoding, memory map, flag semantics
    ├── cpu_schematic.txt               Detailed ASCII CPU architecture diagram
    └── Program_Layout_and_Execution.md Stack frames, recursion walkthrough, memory layout
```

---

## Team Contributions

| Team Member | Contributions |
|---|---|
| **Parth Patel** | Designed the overall CPU architecture, instruction set, register layout, memory map, and MMIO scheme. Implemented all seven emulator modules: `cpu.c` (register file), `alu.c` (arithmetic and logic with full flag updates), `control.c` (Fetch/Decode/Execute loop for all 16 opcodes, silent and verbose modes), `bus.c` (memory routing), `memory.c` (64K array, file loader, hex dump), `io.c` (MMIO devices), `main.c` (CLI). Wrote `docs/CPU_Design.md` (full ISA documentation) and `docs/cpu_schematic.txt`. |
| **Harshitha Vadavalli** | Implemented the two-pass assembler: `assembler.c` (full label resolution, instruction encoding for all 16 opcodes and 3 pseudo-instructions, `.word` directive, hex/decimal/char literals, indirect addressing), `assembler.h`, `asm_main.c` (CLI). Wrote all five assembly programs: `hello.asm`, `fibonacci.asm`, `timer.asm`, `multiply.asm`, `primes.asm`. Wrote `docs/Program_Layout_and_Execution.md`. Updated `Makefile` with `programs`, `test`, and `clean` targets. Wrote `README.md`. Managed GitHub repository. |

---

## Demo Video

Watch the Fibonacci Sequence demo here: **[ADD YOUTUBE LINK HERE]**

The video demonstrates: building the project from source, running all five programs, watching the Fetch/Decode/Execute trace in verbose mode, and verifying the Fibonacci memory dump.

---

## Quick Reference

```bash
cd CPU-Design/src

make                                              # build cpu and asm
make programs                                     # assemble all .asm files
make test                                         # run all 7 automated tests
make clean                                        # remove all build artifacts

./cpu run     ../programs/hello.bin               # Hello, World
./cpu run     ../programs/fibonacci.bin           # 0 1 1 2 3 5 8 13 21 34
./cpu run     ../programs/timer.bin               # [TIMER] 100 ... [TIMER] 1
./cpu run     ../programs/multiply.bin            # 42
./cpu run     ../programs/primes.bin              # 2 3 5 7 11 13 17 19 23 29
./cpu verbose ../programs/timer.bin               # cycle-by-cycle trace
./cpu dump    ../programs/fibonacci.bin 0x8000 0x8009   # memory dump
```

---

*EduCore16 — CMPE 220 System Software — San Jose State University — Spring 2026*
