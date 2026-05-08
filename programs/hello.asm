; =========================================================================
; Hello, World
; =========================================================================
;
; Prints "Hello, World" one character at a time by writing each ASCII
; code to the character output device at memory address 0xFF00.
;
; How it works:
;   The string is stored in memory right after the code as raw numbers
;   (72 = 'H', 101 = 'e', etc.). The program walks through those numbers
;   one by one and sends each to the output device. When it hits a 0
;   (the null terminator), it stops.
;
; Run with:  ./cpu run hello.bin
; Expected:  Hello, World
; =========================================================================

        MOVI  R0, #string   ; R0 = address of the first character in memory
        MOVI  R1, #0xFF00   ; R1 = address of the character output device

loop:   LOAD  R2, [R0]      ; R2 = next character from memory (sets Z=1 if it's 0)
        BEQ   done          ; if the character is 0 (null terminator), we're done
        STORE [R1], R2      ; send the character to the output device -- prints it
        ADDI  R0, #1        ; move R0 forward to point at the next character
        JMP   loop          ; go back and process the next character

done:   HALT                ; stop execution

; The string stored in memory as ASCII codes, ending with 0 to mark the end.
; H=72  e=101  l=108  l=108  o=111  ,=44  space=32
; W=87  o=111  r=114  l=108  d=100  newline=10  end=0
string: .word 72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100, 10, 0
