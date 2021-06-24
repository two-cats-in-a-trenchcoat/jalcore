mov $C000, ix
mov 2048, rf  ;  set referesh frequency
mov 1, r1 ; step

drawer:
    mov r0, [ix]  ;  set pixel
    inc ix
    add r1, r0

    cmp $0000, ix
    jmp %10000001, drawer

; reset
mov $C000, ix
mov 0, r0
add 1, r1 ; inc step
jmp $80, drawer

loop:
    jmp $80, loop  ;  unconditional jump

or %00010000, s0