mov $C000, ix

drawer:
    inc ix
    mov %11100011, [ix]  ;  set pixel

    cmp $FFFF, ix
    jmp %10000001, drawer

or %00010000, s0