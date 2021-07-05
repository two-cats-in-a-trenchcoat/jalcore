MACRO bc %r0: ; branch if carry bit
    jmp %00000000, %r0
ENDMACRO

MACRO bnc %r0: ; branch if not carry bit
    jmp %10000000, %r0
ENDMACRO

MACRO beq %r0:
    jmp 1, %r0
ENDMACRO

mov $C002, ix

blank:
    mov $FF, [ix]  ;  set pixel
    add 1, ix

    cmp $C000, ix
    bnc blank

; reset
rdw
mov $C000, ix
mov 2, sp
iter:
    ; sp is loop counter
    
    mov $C000, ix 
    
    add sp, ix
    cmp $C000, ix
    bc next ; if number overflowed then skip
    
    cmp [ix], 0
    beq next ; skip if number is already eliminated
    
    add sp, ix
    cmp $C000, ix
    bc next ; if number overflowed then skip
    
    clearfactors:
        mov 0, [ix]
        
        add sp, ix
        cmp $C000, ix
        jmp %10000000, clearfactors
    
    next:
    
    add 1, sp
    cmp 0, sp
    jmp %10000001, iter

rdw

loop:
    jmp 7, loop  ;  unconditional jump

or %00010000, s0