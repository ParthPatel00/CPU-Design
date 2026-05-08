; =========================================================================
; Prime Numbers — Trial Division
; =========================================================================
;
; Finds and prints the first 10 prime numbers: 2 3 5 7 11 13 17 19 23 29
;
; Algorithm (trial division):
;   For each candidate n starting at 2:
;     For each divisor d from 2 up to (but not including) n:
;       Compute n mod d using repeated subtraction
;       If n mod d == 0  ->  d divides n evenly, so n is NOT prime; skip to n+1
;     If no d divided n  ->  n IS prime; print it, increment count
;   Stop when count reaches 10.
;
; EduCore16 only has BEQ (branch if Zero), so two tricks are used:
;
;   Trick 1 — detect d == n (loop termination):
;     CMP R1, R0  sets Z=1 when d == n  ->  BEQ is_prime
;
;   Trick 2 — detect remainder underflow in the mod loop:
;     After SUB R2, R1 (remainder -= d), if the result wraps around
;     (i.e. remainder < d), bit 15 of R2 will be 1.
;     AND R7, R2  (where R7 = 0x8000) extracts bit 15 into R7.
;     If R7 == 0 (Z=1) -> bit 15 was clear -> remainder still >= 0 -> keep subtracting.
;     If R7 != 0 (Z=0) -> bit 15 was set  -> subtraction underflowed -> n mod d != 0.
;
; Registers:
;   R0  candidate n       R1  trial divisor d
;   R2  mod remainder     R3  prime count
;   R4  target count (10) R5  IO address (0xFF01)
;   R6  scratch           R7  scratch / bit-mask
;
; =========================================================================

        MOVI  R0, #2           ; n = 2  (first candidate)
        MOVI  R3, #0           ; count = 0 (primes printed so far)
        MOVI  R4, #10          ; stop after printing this many primes
        MOVI  R5, #0xFF01      ; number output MMIO address

; -----------------------------------------------------------------------
; Outer loop: test each candidate n for primality
; -----------------------------------------------------------------------
next_n:
        MOVI  R1, #2           ; reset trial divisor d = 2

; -----------------------------------------------------------------------
; Inner loop: try divisor d against candidate n
; -----------------------------------------------------------------------
try_d:
        ; If d has reached n, then no divisor in [2, n-1] divided n -> PRIME
        CMP   R1, R0           ; compare d and n  (d - n)
        BEQ   is_prime         ; d == n -> all divisors failed -> n is prime

        ; --- Compute n mod d by repeated subtraction ---
        MOV   R2, R0           ; remainder = n

mod_loop:
        SUB   R2, R1           ; remainder -= d
        BEQ   not_prime        ; remainder == 0  ->  d divides n  ->  not prime

        ; Check if bit 15 of remainder is set (subtraction underflowed)
        ; If yes, the true remainder is nonzero and n is not divisible by d.
        MOVI  R7, #0x8000      ; bit-15 mask
        AND   R7, R2           ; R7 = remainder & 0x8000  (isolates bit 15)
        BEQ   mod_loop         ; R7 == 0  ->  bit 15 clear  ->  keep subtracting

        ; Bit 15 was set: remainder wrapped below zero -> n mod d != 0
        ADDI  R1, #1           ; d = d + 1  (try next divisor)
        JMP   try_d

; -----------------------------------------------------------------------
; n is composite: d divided it exactly  ->  skip to n + 1
; -----------------------------------------------------------------------
not_prime:
        ADDI  R0, #1           ; n = n + 1
        JMP   next_n

; -----------------------------------------------------------------------
; n is prime: print it, check if we have found enough
; -----------------------------------------------------------------------
is_prime:
        STORE [R5], R0         ; print n to number output (0xFF01)
        ADDI  R3, #1           ; count = count + 1
        CMP   R3, R4           ; compare count to target (10)
        BEQ   done             ; count == 10  ->  finished
        ADDI  R0, #1           ; n = n + 1  (next candidate)
        JMP   next_n

; -----------------------------------------------------------------------
; All 10 primes printed
; -----------------------------------------------------------------------
done:
        MOVI  R6, #0xFF00      ; character output MMIO address
        MOVI  R7, #10          ; newline (ASCII 10)
        STORE [R6], R7         ; print newline
        HALT
