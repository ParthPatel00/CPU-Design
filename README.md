# EduCore16 — 16-bit Software CPU in C

**Course:** CMPE 220 — System Software | San Jose State University | Spring 2026

**Team:** Group 6 — Harshitha Vadavalli & Parth Patel

**GitHub:** https://github.com/ParthPatel00/CPU-Design

---

## At a Glance

| What | Details |
|---|---|
| Language | C (standard C11, no external libraries) |
| CPU | 16-bit, 8 general-purpose registers, 16 instructions |
| Assembler | Two-pass, supports labels, pseudo-instructions, hex/decimal literals |
| Programs | Hello World, Fibonacci, Timer, Recursive Multiply, Prime Numbers |
| Tests | 7 automated tests — all passing |
| Platforms | macOS, Linux |

---

## Requirements

- macOS or Linux
- GCC
- Make
- Git

---

## How to Build and Run — First Time

> **Quick Start — Run Everything in One Command**
>
> If you would like to build the project and see all programs running immediately, the following three commands are all you need:
>
> ```bash
> git clone https://github.com/ParthPatel00/CPU-Design.git
> cd CPU-Design/src
> make demo-part1    # Part 1: builds, assembles, and runs all programs
> make demo-part2    # Part 2: recursive multiply with full stack trace
> ```
>
> **Prefer to run each step individually?** Continue with the numbered steps below.

---

### 1. Clone

```bash
git clone https://github.com/ParthPatel00/CPU-Design.git
cd CPU-Design/src
```

### 2. Build

```bash
make
```

Compiles everything and produces two executables in `src/`:
- `cpu` — the emulator
- `asm` — the assembler

> If you see `make: Nothing to be done for 'all'` — that is normal, everything is already compiled. Skip to step 3.
> To force a full rebuild from scratch: `make clean` then `make`.

### 3. Assemble all programs

```bash
make programs
```

Runs the assembler on all five `.asm` files and writes `.bin` files to `programs/`.

### 4. Run the programs

```bash
./cpu run ../programs/hello.bin
```
```
Hello, World
```

```bash
./cpu run ../programs/fibonacci.bin
```
```
0 1 1 2 3 5 8 13 21 34
```

```bash
./cpu run ../programs/timer.bin
```
```
[TIMER] 100
[TIMER] 99
...
[TIMER] 1
```

```bash
./cpu run ../programs/multiply.bin
```
```
42
```

```bash
./cpu run ../programs/primes.bin
```
```
2 3 5 7 11 13 17 19 23 29
```

### 5. Run everything in one command

**Part 1 — CPU Design:**
```bash
make demo-part1
```
Builds, assembles, and runs all five programs showing actual output + Fibonacci memory dump. No other commands needed.

**Part 2 — Program Layout and Execution:**
```bash
make demo-part2
```
Runs recursive multiply, shows full stack pointer trace per cycle, and prints the final CPU state confirming R0 = 42 and stack fully unwound.

### 6. Run all automated tests

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

## Special Modes

### Verbose — See Every Clock Cycle (Fetch / Decode / Execute)

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

Every cycle shows FETCH (raw instruction), DECODE (opcode + registers), EXECUTE (full CPU state).

### Memory Dump — Inspect RAM

```bash
./cpu dump ../programs/fibonacci.bin 0x8000 0x8009
```

```
--- Memory Dump [0x8000 - 0x8009] ---
ADDR    +0      +1      +2      +3      +4      +5      +6      +7
0x8000  0x0000  0x0001  0x0001  0x0002  0x0003  0x0005  0x0008  0x000D
0x8008  0x0015  0x0022
```

The 10 Fibonacci numbers stored in RAM at addresses 0x8000–0x8009.

### Clean Build

```bash
make clean
```

Removes all compiled files and `.bin` files so everything can be rebuilt fresh.

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
└─────────┼─────────────────────────────────────────────┘
          │ System Bus (16-bit address / 16-bit data)
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
| `0x0000 – 0x7FFF` | Program space — code loaded and executed here |
| `0x8000 – 0xFEFF` | RAM + Stack — stack grows downward from 0xFEFF |
| `0xFF00 – 0xFFFF` | Memory-Mapped I/O — writes go to output devices |

### Registers

| Register | Width | Purpose |
|---|---|---|
| R0 – R7 | 16-bit | General purpose |
| PC | 16-bit | Program Counter |
| SP | 16-bit | Stack Pointer — starts at 0xFEFF, grows downward |
| FLAGS | 16-bit | Z (zero), N (negative), C (carry), V (overflow) |

---

## Instruction Set — 16 Instructions

| Opcode | Instruction | What It Does |
|---|---|---|
| `0x0` | `MOV`   | Copy register to register |
| `0x1` | `MOVI`  | Load 16-bit constant into register |
| `0x2` | `LOAD`  | Read memory into register |
| `0x3` | `STORE` | Write register to memory or I/O device |
| `0x4` | `ADD`   | Add two registers |
| `0x5` | `ADDI`  | Add signed 6-bit immediate to register |
| `0x6` | `SUB`   | Subtract two registers |
| `0x7` | `AND`   | Bitwise AND |
| `0x8` | `OR`    | Bitwise OR |
| `0x9` | `XOR`   | Bitwise XOR |
| `0xA` | `CMP`   | Compare — sets FLAGS, no writeback |
| `0xB` | `JMP`   | Unconditional jump |
| `0xC` | `BEQ`   | Branch if Z flag = 1 |
| `0xD` | `CALL`  | Call function — push return address, jump |
| `0xE` | `PUSH`  | Push register onto stack |
| `0xF` | `POP`   | Pop from stack into register |

**Pseudo-instructions** (assembler expands automatically):

| Write | Expands To | Purpose |
|---|---|---|
| `HALT` | `JMP 0` | Stop the CPU |
| `SUBI Rd, #n` | `ADDI Rd, #(-n)` | Subtract a constant |
| `RET` | `POP R7` + `JMP R7` | Return from function |

---

## Project File Structure

```
CPU-Design/
├── src/
│   ├── Makefile           make / make programs / make test / make clean
│   ├── cpu.h / cpu.c      Register file (R0–R7, PC, SP, FLAGS)
│   ├── alu.h / alu.c      ALU — ADD, SUB, AND, OR, XOR, CMP + flags
│   ├── control.h / .c     Fetch / Decode / Execute loop — all 16 opcodes
│   ├── bus.h / bus.c      Routes reads/writes between CPU, memory, I/O
│   ├── memory.h / .c      64K word array, binary loader, hex dump
│   ├── io.h / io.c        Memory-mapped I/O devices
│   ├── assembler.h / .c   Two-pass assembler — .asm → .bin
│   ├── asm_main.c         Assembler entry point
│   └── main.c             Emulator entry point (run / verbose / dump)
├── programs/
│   ├── hello.asm/.bin     Hello, World
│   ├── fibonacci.asm/.bin First 10 Fibonacci numbers
│   ├── timer.asm/.bin     Countdown 100 → 1
│   ├── multiply.asm/.bin  Recursive multiply(6,7) = 42
│   └── primes.asm/.bin    First 10 prime numbers
└── docs/
    ├── CPU_Design.md                        Full ISA and encoding reference
    ├── cpu_schematic.txt                    CPU architecture diagram
    ├── Program_Layout_and_Execution.md      Stack frames and recursion walkthrough
    ├── Report_Submission1_CPU_Design.md     Part 1 project report
    └── Report_Submission2_Program_Layout.md Part 2 project report
```

---

## Team Contributions

| Team Member | Contributions |
|---|---|
| **Parth Patel** | Designed CPU architecture, ISA, memory map, and MMIO scheme. Implemented all seven emulator modules: `cpu.c`, `alu.c`, `control.c`, `bus.c`, `memory.c`, `io.c`, `main.c`. Wrote `docs/CPU_Design.md` and `docs/cpu_schematic.txt`. |
| **Harshitha Vadavalli** | Implemented two-pass assembler: `assembler.c`, `assembler.h`, `asm_main.c`. Wrote all five assembly programs: `hello.asm`, `fibonacci.asm`, `timer.asm`, `multiply.asm`, `primes.asm`. Wrote both project reports, `docs/Program_Layout_and_Execution.md`, updated `Makefile`, wrote `README.md`, managed GitHub repository. |

---

## Demo Videos

**Part 1 — CPU Design (Fibonacci):** [ADD YOUTUBE LINK HERE]

**Part 2 — Program Layout & Execution (Recursive Multiply):** [ADD YOUTUBE LINK HERE]

---

## Quick Reference

```bash
cd CPU-Design/src

make                                                   # build
make programs                                          # assemble all programs
make demo-part1                                        # Part 1 — run all programs with output
make demo-part2                                        # Part 2 — recursive multiply + stack trace
make test                                              # run all 7 automated tests
make clean                                             # clean build files

./cpu run     ../programs/hello.bin                    # Hello, World
./cpu run     ../programs/fibonacci.bin                # 0 1 1 2 3 5 8 13 21 34
./cpu run     ../programs/timer.bin                    # [TIMER] 100 ... [TIMER] 1
./cpu run     ../programs/multiply.bin                 # 42
./cpu run     ../programs/primes.bin                   # 2 3 5 7 11 13 17 19 23 29
./cpu verbose ../programs/timer.bin                    # cycle-by-cycle trace
./cpu dump    ../programs/fibonacci.bin 0x8000 0x8009  # memory dump
```

---

*EduCore16 — CMPE 220 System Software — San Jose State University — Spring 2026*
