ORG 8000H                   ; Program origin
START:  MOV A, B            ; Move B to A  
        ADD 0FFH            ; Add hex number
        STA RESULT          ; Store result
        MVI B, 'X'          ; Load character
        JMP START           ; Infinite loop
RESULT: DB 0                ; Data storage
        END
