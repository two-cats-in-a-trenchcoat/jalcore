mov $C000, ix
mov $1000, rf  ;  set referesh frequency
mov 1, r1 ; step

drawer:
    mov r0, [ix]  ;  set pixel
    add 1, ix
    add r1, r0

    cmp $C000, ix
    jmp %10000000, drawer

; reset
mov $C000, ix
mov 0, r0
add 1, r1
jmp 7, drawer

loop:
    jmp 7, loop  ;  unconditional jump

or %00010000, s0