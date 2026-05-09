; =========================================================================
; Fibonacci Sequence
; =========================================================================
;
; Computes the first 10 Fibonacci numbers, prints each one, and also
; saves them into RAM starting at address 0x8000 so you can inspect
; them with the memory dump command.
;
; How it works:
;   A Fibonacci sequence starts with 0 and 1. Each next number is the
;   sum of the previous two: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34 ...
;   We keep two registers (R0 = current, R1 = next) and shuffle them
;   forward each loop iteration using a temporary register (R4).
;
; Run with:   ./cpu run fibonacci.bin
; Expected:   0 1 1 2 3 5 8 13 21 34
;
; Inspect RAM after running:
;   ./cpu dump fibonacci.bin 0x8000 0x8009
; =========================================================================

        MOVI  R0, #0        ; R0 = current Fibonacci number, starting at F(0) = 0
        MOVI  R1, #1        ; R1 = next Fibonacci number, starting at F(1) = 1
        MOVI  R2, #0x8000   ; R2 = RAM address where we will save each result
        MOVI  R3, #10       ; R3 = how many numbers left to print (countdown from 10)
        MOVI  R5, #0xFF01   ; R5 = address of the number output device
        MOVI  R6, #0xFF00   ; R6 = address of the character output device
        MOVI  R7, #32       ; R7 = ASCII 32, which is a space character

loop:   STORE [R5], R0      ; print the current Fibonacci number
        STORE [R6], R7      ; print a space after the number
        STORE [R2], R0      ; also save it to RAM at address R2
        MOV   R4, R1        ; R4 = hold a copy of R1 before we overwrite it
        ADD   R1, R0        ; R1 = R0 + R1  (the next Fibonacci number)
        MOV   R0, R4        ; R0 = old R1   (step R0 forward to the next value)
        ADDI  R2, #1        ; move the RAM save address forward by one slot
        ADDI  R3, #-1       ; decrement the counter (sets Z=1 when it reaches 0)
        BEQ   done          ; if counter hit 0, we have printed all 10 numbers
        JMP   loop          ; otherwise loop back for the next number

done:   MOVI  R7, #10       ; R7 = ASCII 10 = newline character
        STORE [R6], R7      ; print a newline at the end
        HALT                ; done -- stop execution
