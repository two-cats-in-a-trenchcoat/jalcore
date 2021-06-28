jmp 7, start

fill:
    pop ix ; temp store return address whilst getting other parameters
    pop r0
    push ix ; push return address again
    mov $C000, ix
    drawer:
        mov r0, [ix]  ;  set pixel
        add 1, ix

        cmp $C000, ix
        jmp %10000000, drawer
    mov $C000, ix
    ret


start:
; Set SP
mov $9FFF, SP

mov 0, r1
loop:
    push r1
    jsr 7, fill ; always jump
    add 1, r1
    jmp 7, loop  ;  unconditional jump