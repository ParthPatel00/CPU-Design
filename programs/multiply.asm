; =========================================================================
; Recursive Multiply: multiply(3, 2) = 6
; =========================================================================
;
; C equivalent (see programs/multiply.c):
;
;   int multiply(int a, int b) {
;       if (b == 0) return 0;
;       return a + multiply(a, b - 1);
;   }
;
;   int main(void) {
;       int result = multiply(3, 2);
;       printf("%d\n", result);    // prints 6
;       return 0;
;   }
;
; Calling convention:
;   R0 = first argument (a), also used for return value
;   R1 = second argument (b)
;   R6 = function address (loaded with MOVI before CALL)
;   R7 = return address scratch (used by RET = POP R7; JMP R7)
;
; =========================================================================


; --------------- main() ---------------
; C: int result = multiply(3, 2);
;    printf("%d\n", result);

        MOVI  R0, #3           ; first argument: a = 3
        MOVI  R1, #2           ; second argument: b = 2
        MOVI  R6, #multiply    ; load the address of multiply() into R6
        CALL  R6               ; jump to multiply, push return address on stack
                                ; after this returns, R0 holds the result

        ; C: printf("%d\n", result) -- we use memory-mapped IO instead
        MOVI  R5, #0xFF01      ; 0xFF01 = number output IO port
        STORE [R5], R0         ; write R0 to the IO port, prints "6"
        MOVI  R5, #0xFF00      ; 0xFF00 = character output IO port
        MOVI  R4, #10          ; ASCII 10 = newline character
        STORE [R5], R4         ; print the newline
        HALT                   ; done, stop the CPU


; --------------- multiply(a, b) ---------------
; C: if (b == 0) return 0;
;    return a + multiply(a, b - 1);
;
; Stack frame per call (3 words):
;    | return address |  pushed by CALL
;    | saved R0 (a)   |  pushed by PUSH R0
;    | saved R1 (b)   |  pushed by PUSH R1  <-- SP

multiply:
        ; C: if (b == 0)
        ADDI  R1, #0           ; adds 0 to b, doesn't change value but sets Z flag
        BEQ   mul_base         ; if Z flag is set (b == 0), skip to base case

        ; save a and b so we can restore them after the recursive call
        PUSH  R0               ; push a onto the stack
        PUSH  R1               ; push b onto the stack

        ; C: multiply(a, b - 1)
        ADDI  R1, #-1          ; b = b - 1 (R0 still has a, unchanged)
        MOVI  R6, #multiply    ; reload the function address
        CALL  R6               ; recurse: R0 = multiply(a, b - 1)

        ; back from recursion, restore our saved a and b
        POP   R1               ; pop b back off the stack
        POP   R2               ; pop a into R2 (can't use R0, it has the result)

        ; C: return a + multiply(a, b - 1)
        ADD   R0, R2           ; R0 = recursive result + a
        RET                    ; pop return address, jump back to caller

        ; C: return 0
mul_base:
        MOVI  R0, #0           ; return value = 0
        RET                    ; pop return address, jump back to caller
