mov $C002, ix

blank:
    mov $FF, [ix]  ;  set pixel
    add 1, ix

    cmp $C000, ix
    jmp %10000000, blank

; reset
rdw
mov $C000, ix
mov 2, r0
iter:
    ; r0 is loop counter
    
    mov $C000, ix
    add r0, ix
    add r0, ix
    clearfactors:
        mov 0, [ix]
        
        add r0, ix
        cmp $C000, ix
        jmp %10000000, clearfactors

    rdw
    add 1, r0
    cmp 0, r0
    jmp %10000001, iter


loop:
    jmp 7, loop  ;  unconditional jump

or %00010000, s0