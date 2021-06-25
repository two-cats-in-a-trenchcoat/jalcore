mov $C000, ix
mov $1000, rf  ;  set referesh frequency

drawer:
    mov r0, [ix]  ;  set pixel
    add 1, ix
    add 1, r0

    cmp $C000, ix
    jmp %10000000, drawer

; reset
mov $C000, ix
mov 0, r0
jmp 7, drawer

loop:
    jmp 7, loop  ;  unconditional jump

or %00010000, s0