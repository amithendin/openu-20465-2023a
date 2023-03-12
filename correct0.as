.entry LENGTH
.extern W
MAIN:   mov r3 ,LENGTH
LOOP:   jmp L3(#-1,r6)
        mcr ml
            sub r1, r4
            bne END
        endmcr
        prn #-5
        bne W(r4,r5)
        ml
L1:     inc K
.entry LOOP
        bne LOOP(K,W)
END:    stop
STR:    .string "abcdef"
LENGTH: .data 6,-9,15
K:      .data 22
.extern L3