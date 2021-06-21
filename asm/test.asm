mov 10, R0
loop:
    dec R0
    jmp %10000001, loop
or %00010000, S0