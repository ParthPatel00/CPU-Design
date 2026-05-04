# Program Layout and Execution

## How to Build and Run

```bash
cd src
make
./asm ../programs/multiply.asm ../programs/multiply.bin
./cpu run ../programs/multiply.bin          # prints: 42
./cpu verbose ../programs/multiply.bin      # full cycle-by-cycle trace
```

---

## The Program

Recursive multiplication via repeated addition. The C equivalent:

```c
int multiply(int a, int b) {
    if (b == 0) return 0;
    return a + multiply(a, b - 1);
}

int main(void) {
    printf("%d\n", multiply(6, 7));    // prints 42
}
```

### Calling Convention

| Role           | Register | Notes                                |
|----------------|----------|--------------------------------------|
| Argument 1     | R0       | Also holds the return value          |
| Argument 2     | R1       |                                      |
| Function addr  | R6       | Loaded with MOVI before CALL         |
| Return scratch | R7       | Used internally by RET (POP R7; JMP R7) |

The callee saves R0 and R1 on the stack before recursing, restores them after.

---

## Memory Layout of the Executable

After assembling and loading `multiply.bin`, the binary is placed at address 0x0000:

```
Address   Hex      Instruction              Section
-------   ------   ----------------------   -----------------
0x0000    0x1000   MOVI R0, #6              Main program
0x0001    0x0006     (immediate: 6)            |
0x0002    0x1200   MOVI R1, #7                |
0x0003    0x0007     (immediate: 7)            |
0x0004    0x1C00   MOVI R6, #multiply         |
0x0005    0x0010     (immediate: 0x0010)       |
0x0006    0xD180   CALL R6                     |
0x0007    0x1A00   MOVI R5, #0xFF01           |
0x0008    0xFF01     (immediate: 0xFF01)       |
0x0009    0x3A00   STORE [R5], R0             |
0x000A    0x1A00   MOVI R5, #0xFF00           |
0x000B    0xFF00     (immediate: 0xFF00)       |
0x000C    0x1800   MOVI R4, #10               |
0x000D    0x000A     (immediate: 10)           |
0x000E    0x3B00   STORE [R5], R4             |
0x000F    0xB000   HALT                        |
                                            -----------------
0x0010    0x5200   ADDI R1, #0               multiply()
0x0011    0xC00C   BEQ +12 (-> mul_base)       |
0x0012    0xE000   PUSH R0                     |
0x0013    0xE040   PUSH R1                     |
0x0014    0x523F   ADDI R1, #-1                |
0x0015    0x1C00   MOVI R6, #multiply          |
0x0016    0x0010     (immediate: 0x0010)        |
0x0017    0xD180   CALL R6                      |
0x0018    0xF200   POP R1                       |
0x0019    0xF400   POP R2                       |
0x001A    0x4080   ADD R0, R2                   |
0x001B    0xFE00   POP R7   (RET part 1)        |
0x001C    0xB9C0   JMP R7   (RET part 2)        |
                                            -----------------
0x001D    0x1000   MOVI R0, #0               mul_base
0x001E    0x0000     (immediate: 0)             |
0x001F    0xFE00   POP R7   (RET part 1)        |
0x0020    0xB9C0   JMP R7   (RET part 2)        |
```

The rest of the 64K address space:

```
0x0021 - 0x7FFF   Unused program space
0x8000 - 0xFEFF   RAM (stack lives here, growing downward from 0xFEFF)
0xFF00 - 0xFFFF   Memory-mapped IO
```

---

## How Function Calls Are Handled

### CALL R6

Two things happen in one cycle:

1. SP is decremented, and the current PC (the return address, already pointing past the CALL instruction) is written to `mem[SP]`.
2. PC is set to the value in R6 (the function address).

```
Before CALL R6:                After CALL R6:
  PC = 0x0007                    PC = 0x0010 (multiply)
  SP = 0xFEFF                    SP = 0xFEFE

  Stack:                         Stack:
  0xFEFE [       ]               0xFEFE [0x0007] <-- SP  (return address)
  0xFEFF [       ] <-- SP        0xFEFF [       ]
```

### RET (POP R7; JMP R7)

The assembler expands RET into two instructions:

1. `POP R7` reads the return address from `mem[SP]` into R7, then increments SP.
2. `JMP R7` sets PC to R7 (register-indirect jump).

```
Before RET:                      After RET:
  SP = 0xFEFE                     SP = 0xFEFF
  mem[0xFEFE] = 0x0007            PC = 0x0007  (back in caller)
```

---

## How Recursion Is Carried Out

### Stack Frame Per Call

Each call to `multiply` builds a frame of 3 words on the stack:

```
  | return address |   pushed by CALL
  | saved R0 (a)   |   pushed by PUSH R0
  | saved R1 (b)   |   pushed by PUSH R1    <-- SP
```

The function saves R0 and R1 so that each recursion level works with its own copy of `a` and `b`. After the recursive call returns, those values are restored and combined with the result.

### Traced Example: multiply(3, 2) = 6

**Main calls multiply(3, 2):**

```
CALL R6 at 0x0006 -> push return addr 0x0007, jump to 0x0010
Stack: [ 0x0007 ]   SP=0xFEFE
```

**Call 1: multiply(a=3, b=2)**

```
b != 0, so save state and recurse:
  PUSH R0 (a=3), PUSH R1 (b=2)
  Stack: [ 2 | 3 | 0x0007 ]   SP=0xFEFC

  ADDI R1, #-1  ->  R1 = 1
  CALL R6       ->  push return addr 0x0018
  Stack: [ 0x0018 | 2 | 3 | 0x0007 ]   SP=0xFEFB
```

**Call 2: multiply(a=3, b=1)**

```
b != 0, so save state and recurse:
  PUSH R0 (a=3), PUSH R1 (b=1)
  Stack: [ 1 | 3 | 0x0018 | 2 | 3 | 0x0007 ]   SP=0xFEF9

  ADDI R1, #-1  ->  R1 = 0
  CALL R6       ->  push return addr 0x0018
  Stack: [ 0x0018 | 1 | 3 | 0x0018 | 2 | 3 | 0x0007 ]   SP=0xFEF8
```

**Call 3: multiply(a=3, b=0)  -- BASE CASE**

```
b == 0!  MOVI R0, #0.  RET.
  POP R7 = 0x0018, JMP R7.  (returns to call 2)
  Stack: [ 1 | 3 | 0x0018 | 2 | 3 | 0x0007 ]   SP=0xFEF9
  R0 = 0
```

**Unwinding call 2:**

```
POP R1 = 1 (restored b),  POP R2 = 3 (restored a)
Stack: [ 0x0018 | 2 | 3 | 0x0007 ]   SP=0xFEFB

ADD R0, R2  ->  R0 = 0 + 3 = 3
RET  ->  POP R7 = 0x0018, JMP R7.  (returns to call 1)
Stack: [ 2 | 3 | 0x0007 ]   SP=0xFEFC
R0 = 3
```

**Unwinding call 1:**

```
POP R1 = 2 (restored b),  POP R2 = 3 (restored a)
Stack: [ 0x0007 ]   SP=0xFEFE

ADD R0, R2  ->  R0 = 3 + 3 = 6
RET  ->  POP R7 = 0x0007, JMP R7.  (returns to main)
Stack: [ ]   SP=0xFEFF
R0 = 6 = 3 * 2  ✓
```

### Stack Depth Visualization

```
Depth   Stack contents (left = top)                         R0 (return val)
-----   -----------------------------------------------     ---------------
call 1: [ ret | a=3 | b=2 ]
call 2: [ ret | a=3 | b=2 | ret | a=3 | b=1 ]
call 3: [ ret | a=3 | b=2 | ret | a=3 | b=1 | ret ]       (base case)
ret 3:  [ ret | a=3 | b=2 | ret | a=3 | b=1 ]              R0 = 0
ret 2:  [ ret | a=3 | b=2 ]                                 R0 = 0+3 = 3
ret 1:  [ ]                                                  R0 = 3+3 = 6
```

For `multiply(6, 7)`, the recursion goes 7 levels deep. Each level uses 3 stack words (return address + saved a + saved b), plus 1 for the initial CALL from main. Maximum stack usage: 22 words.

### Actual Output

```
$ ./cpu run programs/multiply.bin
42
```

The verbose trace (`./cpu verbose programs/multiply.bin`) shows all 99 cycles with the full CPU state after each instruction, confirming SP moves from 0xFEFF down to 0xFEE9 (deepest recursion) and back to 0xFEFF (fully unwound).
