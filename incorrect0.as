.entry #LENGTH
.extern 7W
MAIN:   mov r3 ,1LENGTH
LOOP:   jmp L(#-1,r6)
        mcr ml
        sub r1 r4
        bne END
        endmcr
        prn #-500000
        bne (r4,r5)
        ml
L1:     inc K
.entry LOOP
        bne LOOP(K,7777777777)
END:    stop
STR:    .string "abcd,ef"
LENGTH: .data 6,-9,15
K:      .data 22
        .extern L3