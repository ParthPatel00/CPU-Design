; =========================================================================
; Timer Countdown
; =========================================================================
;
; Counts down from 100 to 1, displaying each value on the timer output
; device. This program is the best one to use with verbose mode because
; it is simple and short -- you can clearly see each Fetch / Decode /
; Execute cycle as the counter ticks down.
;
; Run with:   ./cpu run timer.bin
; Expected:   [TIMER] 100  [TIMER] 99  ...  [TIMER] 1
;
; To see every instruction cycle in detail:
;   ./cpu verbose timer.bin
; =========================================================================

        MOVI  R0, #100      ; R0 = starting value of the counter (100)
        MOVI  R1, #0xFF02   ; R1 = address of the timer display device

loop:   STORE [R1], R0      ; show the current counter value on the timer display
        ADDI  R0, #-1       ; subtract 1 from the counter (sets Z=1 when it hits 0)
        BEQ   done          ; if the counter just reached 0, stop
        JMP   loop          ; otherwise go back and display the next value

done:   HALT                ; counter reached 0 -- stop execution
