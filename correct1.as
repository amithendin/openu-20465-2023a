.entry LENGTH
.extern W
MAIN:   mov r3 ,LENGTH
LOOP:   jmp L1(#4,r6)
        mcr ml
            sub r1, r4
            bne END 
        endmcr
        prn #-50
        bne LOOP(r4,r3) 
        ml
L1:     inc K
.entry LOOP
        bne LOOP(K,STR)
END:    stop
STR:    .string "abcdef"
LENGTH: .data 6,-9,15
K:      .data 22
.extern L3