# EduCore16 — 16-bit Software CPU in C

**CMPE 220 — System Software | San Jose State University | Spring 2026**

**Team:** Group 6 — Harshitha Vadavalli & Parth Patel

**GitHub:** https://github.com/ParthPatel00/CPU-Design

---

## What This Project Is

A complete **16-bit CPU emulator and assembler** written in C from scratch.

The emulator simulates every hardware component of a real processor — registers, ALU, control unit, bus, 64K memory, and memory-mapped I/O — all in separate source files that mirror physical CPU architecture. The assembler compiles `.asm` source files into binary programs the CPU runs.

Five assembly programs demonstrate the full capability of the CPU: Hello World, Fibonacci Sequence, Timer Countdown, Recursive Multiply, and Prime Numbers.

---

## Requirements

- macOS or Linux
- GCC (any modern version)
- Git
- Make

No external libraries. Pure standard C.

---

## Build and Run — Quick Start

> **Run everything in one command — no individual steps needed:**
>
> ```bash
> git clone https://github.com/ParthPatel00/CPU-Design.git
> cd CPU-Design/src
> make demo-part1    # Part 1 — builds, assembles, and runs all programs
> make demo-part2    # Part 2 — recursive multiply with full stack trace
> ```
>
> This builds the project, assembles all programs, and runs the complete demonstration automatically. No additional commands are required.
>
> If you prefer to run each step individually, continue with the steps below.

---

## Build and Run — Step by Step

### Step 1 — Clone and Build

```bash
git clone https://github.com/ParthPatel00/CPU-Design.git
cd CPU-Design/src
make
```

Output you will see:
```
gcc -Wall -Wextra -std=c11 -g -c -o main.o main.c
...
```
Two executables are created: **`cpu`** (the emulator) and **`asm`** (the assembler).

> **Note:** If you see `make: Nothing to be done for 'all'` — that is normal, the project is already compiled. Proceed to Step 2. To force a clean rebuild: run `make clean` then `make`.

---

### Step 2 — Assemble All Programs

```bash
make programs
```

Output:
```
[ASM] hello.asm       → 24 words
[ASM] fibonacci.asm   → 25 words
[ASM] timer.asm       → 9 words
[ASM] multiply.asm    → 33 words
[ASM] primes.asm      → 35 words
```

---

### Step 3 — Run the Programs

**Hello, World**
```bash
./cpu run ../programs/hello.bin
```
```
Hello, World
```

**Fibonacci Sequence**
```bash
./cpu run ../programs/fibonacci.bin
```
```
0 1 1 2 3 5 8 13 21 34
```

**Timer Countdown**
```bash
./cpu run ../programs/timer.bin
```
```
[TIMER] 100
[TIMER] 99
...
[TIMER] 1
```

**Recursive Multiply — multiply(6, 7) = 42**
```bash
./cpu run ../programs/multiply.bin
```
```
42
```

**Prime Numbers — first 10 primes**
```bash
./cpu run ../programs/primes.bin
```
```
2 3 5 7 11 13 17 19 23 29
```

---

## Fetch / Decode / Execute — Verbose Mode

See every instruction cycle printed step by step:

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

This shows exactly what happens inside the CPU on every clock cycle — FETCH reads the instruction, DECODE extracts the fields, EXECUTE runs it and updates all registers.

---

## Memory Dump — Verify RAM Contents

After running Fibonacci, inspect what was stored in RAM:

```bash
./cpu dump ../programs/fibonacci.bin 0x8000 0x8009
```

```
--- Memory Dump [0x8000 - 0x8009] ---
ADDR    +0      +1      +2      +3      +4      +5      +6      +7
0x8000  0x0000  0x0001  0x0001  0x0002  0x0003  0x0005  0x0008  0x000D
0x8008  0x0015  0x0022
```

The 10 Fibonacci numbers (0, 1, 1, 2, 3, 5, 8, 13, 21, 34) stored in memory at addresses 0x8000–0x8009.

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

## CPU Architecture at a Glance

```
┌─────────────────────────────────────────────────────────┐
│                     EduCore16 CPU                        │
│                                                          │
│  Registers: R0–R7 (general), PC, SP, FLAGS              │
│                                                          │
│  ┌──────────┐   ┌──────────┐   ┌──────────────────┐    │
│  │  Control │   │   ALU    │   │  Register File   │    │
│  │  Unit    │   │ ADD SUB  │   │  R0 R1 R2 R3     │    │
│  │ F/D/E    │   │ AND OR   │   │  R4 R5 R6 R7     │    │
│  │  loop    │   │ XOR CMP  │   │  PC  SP  FLAGS   │    │
│  └────┬─────┘   └──────────┘   └──────────────────┘    │
│       │                                                  │
└───────┼──────────────────────────────────────────────────┘
        │ System Bus (16-bit address / 16-bit data)
        │
   ┌────┴──────────────────────┐
   │                           │
┌──▼──────────┐    ┌───────────▼───────────┐
│   Memory    │    │   Memory-Mapped I/O   │
│  64K words  │    │  0xFF00 → char out    │
│  0x0000–    │    │  0xFF01 → number out  │
│  0xFEFF     │    │  0xFF02 → timer out   │
└─────────────┘    └───────────────────────┘
```

**Memory Map:**

| Range | Purpose |
|---|---|
| `0x0000 – 0x7FFF` | Program space — code loaded and executed here |
| `0x8000 – 0xFEFF` | RAM + Stack — stack grows downward from 0xFEFF |
| `0xFF00 – 0xFFFF` | Memory-Mapped I/O — writes go to output devices |

**Registers:**

| Register | Width | Purpose |
|---|---|---|
| R0 – R7 | 16-bit | General purpose |
| PC | 16-bit | Program Counter — address of next instruction |
| SP | 16-bit | Stack Pointer — starts at 0xFEFF, grows downward |
| FLAGS | 16-bit | Z (zero), N (negative), C (carry), V (overflow) |

---

## Instruction Set — 16 Instructions

| Opcode | Instruction | What It Does |
|---|---|---|
| `0x0` | `MOV`   | Copy register to register |
| `0x1` | `MOVI`  | Load 16-bit constant into register (2-word instruction) |
| `0x2` | `LOAD`  | Read memory into register |
| `0x3` | `STORE` | Write register to memory or I/O device |
| `0x4` | `ADD`   | Add two registers |
| `0x5` | `ADDI`  | Add signed 6-bit immediate (−32 to +31) |
| `0x6` | `SUB`   | Subtract two registers |
| `0x7` | `AND`   | Bitwise AND |
| `0x8` | `OR`    | Bitwise OR |
| `0x9` | `XOR`   | Bitwise XOR |
| `0xA` | `CMP`   | Compare — sets FLAGS, changes nothing |
| `0xB` | `JMP`   | Unconditional jump (PC-relative or register) |
| `0xC` | `BEQ`   | Branch if Zero flag = 1 |
| `0xD` | `CALL`  | Call function — push return address, jump |
| `0xE` | `PUSH`  | Push register onto stack |
| `0xF` | `POP`   | Pop from stack into register |

**Pseudo-instructions** (assembler expands automatically):

| Write This | Assembler Produces | Purpose |
|---|---|---|
| `HALT` | `JMP 0` (self-loop) | Cleanly stops the CPU |
| `SUBI R0, #5` | `ADDI R0, #-5` | Subtract a constant |
| `RET` | `POP R7` + `JMP R7` | Return from a function |

---

## Project File Structure

```
CPU-Design/
├── src/
│   ├── Makefile          make / make programs / make demo-part1 / make demo-part2 / make test / make clean
│   ├── cpu.h / cpu.c     Registers (R0–R7, PC, SP, FLAGS), initialization
│   ├── alu.h / alu.c     All math and logic: ADD, SUB, AND, OR, XOR + flags
│   ├── control.h / .c    Fetch / Decode / Execute loop — the CPU brain
│   ├── bus.h / bus.c     Routes reads/writes between CPU, memory, and I/O
│   ├── memory.h / .c     64K word array, binary file loader, memory dump
│   ├── io.h / io.c       Character, number, and timer output devices
│   ├── assembler.h / .c  Two-pass assembler: source text → binary
│   ├── asm_main.c        Assembler command-line entry point
│   └── main.c            Emulator entry point (run / verbose / dump)
├── programs/
│   ├── hello.asm/.bin     Prints "Hello, World" via memory-mapped character I/O
│   ├── fibonacci.asm/.bin First 10 Fibonacci numbers, printed + stored in RAM
│   ├── timer.asm/.bin     Counts down 100→1 on the timer device
│   ├── multiply.asm/.bin  Recursive multiply(6,7)=42 using CALL/PUSH/POP/RET
│   └── primes.asm/.bin    First 10 prime numbers via trial division
└── docs/
    ├── CPU_Design.md                        Full ISA, encoding, memory map, flag semantics
    ├── cpu_schematic.txt                    Detailed ASCII CPU architecture diagram
    ├── Program_Layout_and_Execution.md      Stack frames, recursion walkthrough, memory layout
    ├── Report_Submission1_CPU_Design.md     Part 1 project report
    └── Report_Submission2_Program_Layout.md Part 2 project report
```

---

## Team Contributions

Both team members contributed equally to this project. The work was divided by component — Parth focused on the CPU hardware simulation and Harshitha focused on the software tools, programs, and documentation.

| Team Member | Specific Contributions |
|---|---|
| **Parth Patel** | Designed the overall CPU architecture, instruction set (ISA), register layout, memory map, and memory-mapped I/O scheme. Implemented all 7 emulator modules: `cpu.c` (register file), `alu.c` (arithmetic and flag logic), `control.c` (fetch/decode/execute loop for all 16 instructions), `bus.c` (memory routing), `memory.c` (64K array, file loader, hex dump), `io.c` (MMIO devices), `main.c` (CLI with run, verbose, and dump modes). Designed the instruction encoding format, flag semantics, and calling convention used by the assembler and programs. Wrote `docs/CPU_Design.md` (full ISA documentation with all encodings) and `docs/cpu_schematic.txt` (detailed CPU architecture diagram). |
| **Harshitha Vadavalli** | Implemented the two-pass assembler: `assembler.c` (full label resolution, instruction encoding for all 16 opcodes + pseudo-instructions), `assembler.h`, `asm_main.c` (CLI). Wrote all 5 assembly programs: `hello.asm`, `fibonacci.asm`, `timer.asm`, `multiply.asm`, `primes.asm`. Built and ran the complete test suite — verified all 7 automated tests pass across all programs including memory dump validation. Wrote `docs/Program_Layout_and_Execution.md` (stack frames, recursion walkthrough, memory layout). Updated `Makefile` with `programs`, `demo-part1`, `demo-part2`, `test`, and `clean` targets. Wrote `README.md`. Wrote both project reports: `docs/Report_Submission1_CPU_Design.md` (Part 1) and `docs/Report_Submission2_Program_Layout.md` (Part 2). Managed GitHub repository. |

---

## Demo Videos

**Part 1 — CPU Design (Fibonacci Sequence):** [ADD YOUTUBE LINK HERE]

**Part 2 — Program Layout & Execution (Recursive Multiply):** [ADD YOUTUBE LINK HERE]

---

## Quick Reference

```bash
cd CPU-Design/src

make                                                    # build cpu and asm
make programs                                           # assemble all .asm files
make demo-part1                                         # Part 1 — run all programs with output
make demo-part2                                         # Part 2 — recursive multiply + stack trace
make test                                               # run all 7 automated tests
make clean                                              # remove all build artifacts

./cpu run     ../programs/hello.bin                     # Hello, World
./cpu run     ../programs/fibonacci.bin                 # 0 1 1 2 3 5 8 13 21 34
./cpu run     ../programs/timer.bin                     # [TIMER] 100 ... [TIMER] 1
./cpu run     ../programs/multiply.bin                  # 42
./cpu run     ../programs/primes.bin                    # 2 3 5 7 11 13 17 19 23 29
./cpu verbose ../programs/timer.bin                     # cycle-by-cycle trace
./cpu dump    ../programs/fibonacci.bin 0x8000 0x8009   # memory dump
```

---

*EduCore16 — CMPE 220 System Software — San Jose State University — Spring 2026*
