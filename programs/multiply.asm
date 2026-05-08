; =========================================================================
; Recursive Multiply
; =========================================================================
;
; C equivalent:
;
;   int multiply(int a, int b) {
;       if (b == 0) return 0;
;       return a + multiply(a, b - 1);
;   }
;
;   int main(void) {
;       int result = multiply(6, 7);
;       printf("%d\n", result);    // prints 42
;       return 0;
;   }
;
; Calling convention:
;   Arguments:     R0 = a (first),  R1 = b (second)
;   Return value:  R0
;   Caller sets up R0 and R1 before CALL. Callee returns result in R0.
;   The function preserves R0 and R1 on the stack before recursing so
;   each stack frame holds its own copy of a and b.
;
; =========================================================================

; --------------- Main Program (driver) ---------------

        MOVI  R0, #6           ; a = 6
        MOVI  R1, #7           ; b = 7
        MOVI  R6, #multiply    ; load function address into R6
        CALL  R6               ; call multiply(6, 7), result returned in R0
        MOVI  R5, #0xFF01      ; number output IO address
        STORE [R5], R0         ; print result (expect 42)
        MOVI  R5, #0xFF00      ; character output IO address
        MOVI  R4, #10          ; newline character
        STORE [R5], R4         ; print newline
        HALT

; --------------- multiply(a=R0, b=R1) -> R0 ---------------
;
; Each recursive call builds a stack frame:
;
;   High address
;    |  ...             |
;    |  return address  |  <-- pushed by CALL
;    |  saved R0 (a)    |  <-- pushed by PUSH R0
;    |  saved R1 (b)    |  <-- pushed by PUSH R1
;    |  ...             |
;   Low address (SP points here)
;
; On return, the frame is unwound by POP R1, POP R2, then RET.

multiply:
        ; Base case check: is b == 0?
        ADDI  R1, #0           ; add 0 to R1: value unchanged, Z flag set if R1 is 0
        BEQ   mul_base         ; if b == 0, jump to base case

        ; --- Recursive case ---

        ; Save current a and b before the recursive call overwrites them
        PUSH  R0               ; save a onto the stack
        PUSH  R1               ; save b onto the stack

        ; Set up arguments for recursive call: multiply(a, b - 1)
        ADDI  R1, #-1          ; b = b - 1  (R0 still holds a, unchanged)
        MOVI  R6, #multiply    ; reload function address into R6
        CALL  R6               ; R0 = multiply(a, b - 1)

        ; Restore saved registers after the recursive call returns
        POP   R1               ; restore b (for stack hygiene)
        POP   R2               ; restore a into R2 (R0 holds the recursive result)

        ; Combine: result = a + multiply(a, b - 1)
        ADD   R0, R2           ; R0 = recursive_result + a
        RET                    ; return to caller

        ; --- Base case: b == 0, return 0 ---

mul_base:
        MOVI  R0, #0           ; return value = 0
        RET                    ; return to caller
