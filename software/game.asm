MACRO hlt:
    or %00010000, s0
ENDMACRO

MACRO bc %r0: ; branch if carry bit
    jmp %00000000, %r0
ENDMACRO

MACRO bnc %r0: ; branch if not carry bit
    jmp %10000000, %r0
ENDMACRO

MACRO jnz %r0: ; jump if not zero bit
    jmp %10000001, %r0
ENDMACRO

MACRO jz %r0: ; jump if zero bit
    jmp %00000001, %r0
ENDMACRO

; PROGRAM
mov $9FFF, sp ; initialize stack pointer
jmp 7, start ; program start

; DATA

color: .data u8, $00
x: .data u8, $00
y: .data u8, $00

w: .data u8, $00
h: .data u8, $00

ball_x: .data u8, 64
ball_y: .data u8, 32
ball_w: .data u8, 9
ball_h: .data u8, 6

ball_vx: .data u8, 1
ball_vy: .data u8, 1

; SUBROUTINES

fill:
    mov $C000, ix
    __drawer1:
        mov color, [ix]  ;  set pixel
        add 1, ix

        cmp $C000, ix
        jmp %10000000, __drawer1
    mov $C000, ix
    ret

draw_box:
    mov x, r1
    mov y, r2
    mov h, r3
    ; this assumes w and h are not 0
    __hloop1:
        ; draw this line
        mov w, r4
        mov r1, x
        __wloop1:
            add 1, x
            jsr 7, draw_dot
            dec r4
            jnz __wloop1
        add 1, y
        dec r3
        jnz __hloop1
    
    ret

draw_dot:
    mov $C000, ix
    
    ; add y rows
    mov y, r0
    cmp 0, r0
    jz __skip1
    __addy1:
        add 128, ix
        dec r0
        jnz __addy1
    __skip1:
    add x, ix

    mov color, [ix]
    ret

process_ball:
    ; X velocity
    add ball_vx, ball_x
    
    ; collision with left
    
    cmp ball_x, 127
    IF 0: ; ball_x > 127
        cmp ball_vx, 127
        IF 0:  ; left <-
            mov 0, ball_x
        ENDIF
        
        ; invert velocity
        mov 0, r1
        sub ball_vx, r1
        mov r1, ball_vx
    ENDIF

    ; collision with right

    mov ball_x, r0
    add ball_w, r0
    cmp r0, 127
    IF 0:
        cmp 127, ball_vx
        IF 0:  ; right ->
            mov 127, ball_x
            sub ball_w, ball_x
        ENDIF
        ; invert velocity
        mov 0, r1
        sub ball_vx, r1
        mov r1, ball_vx
    ENDIF



    ; y velocity
    add ball_vy, ball_y
    
    ; collision with top
    
    cmp ball_y, 127
    IF 0: ; ball_y > 127
        cmp ball_vy, 127
        IF 0:  ; left <-
            mov 0, ball_y
        ENDIF
        
        ; invert velocity
        mov 0, r1
        sub ball_vy, r1
        mov r1, ball_vy
    ENDIF

    ; collision with bottom

    mov ball_y, r0
    add ball_h, r0
    cmp r0, 127
    IF 0:
        cmp 127, ball_vy
        IF 0:  ; right ->
            mov 127, ball_y
            sub ball_h, ball_y
        ENDIF
        ; invert velocity
        mov 0, r1
        sub ball_vy, r1
        mov r1, ball_vy
    ENDIF
    

    ret

; START
start:


main:
    ; physics step
    jsr 7, process_ball



    ; render
    mov $00, color
    jsr 7, fill
    mov $FF, color
    mov ball_x, x
    mov ball_y, y
    mov ball_w, w
    mov ball_h, h
    jsr 7, draw_box
    rdw

    jmp 7, main

hlt