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
or %00100000, s0 ; enable interrupts
mov $7000, [$9FFE]
jmp 7, start ; program start

; DATA

color: .data u8, $00
x: .data u8, $00
y: .data u8, $00

w: .data u8, $00
h: .data u8, $00

ball_x: .data u8, 64
ball_y: .data u8, 64
ball_w: .data u8, 2
ball_h: .data u8, 2

ball_vx: .data u8, 1
ball_vy: .data u8, 0

control1_x: .data u8, 5
control1_y: .data u8, 62
control1_w: .data u8, 1
control1_h: .data u8, 10
control1_half: .data u8, 0
control1_vy: .data u8, 0

paddle_step: .data u8, 1
inverse_paddle_step: .data u8, 0

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
    jsr 7, align_to
    mov x, r1
    mov y, r2
    mov h, r3
    ; this assumes w and h are not 0
    __hloop1:
        mov w, r4
        __wloop1:
            inc ix
            mov color, [ix]
            dec r4
            jnz __wloop1
        sub w, ix
        add 128, ix
        dec r3
        jnz __hloop1
    
    ret

align_to:
    mov $C000, ix
    mov y, r0
    cmp 0, r0
    jz __skip1
    __addy1:
        add 128, ix
        dec r0
        jnz __addy1
    __skip1:
    add x, ix
    ret

draw_dot:
    jsr 7, align_to

    mov color, [ix]
    ret

physics_step:
    ; paddle/control1

    add control1_vy, control1_y

    cmp control1_y, 127
    IF 0: ; top
        mov 0, control1_y
    ENDIF
    
    cmp control1_y, 118
    IF 0: ; bottom
        mov 118, control1_y
    ENDIF

    ; ball

    ; X velocity
    add ball_vx, ball_x

    ; collision check with control1
    cmp control1_x, ball_x
    IF 0: ; control1_x > ball_x
        cmp ball_vx, 127
        IF 0:  ; left <-
            mov ball_y, r0
            sub control1_y, r0
            cmp control1_h, r0
            IF 0: ; if ball_y is in paddle
                mov control1_x, ball_x
                mov 1, ball_vx
                
                cmp r0, control1_half
                IF 1: ; exactly middle
                    mov 0, ball_vy
                    jmp 7, __vy_done1
                ENDIF
                IF 0: ; bottom half
                    mov 1, ball_vy
                    jmp 7, __vy_done1
                ENDIF
                mov 255, ball_vy ; -1
                __vy_done1:
            ENDIF
        ENDIF
    ENDIF
    
    ; collision with left
    
    cmp ball_x, 127
    IF 0: ; ball_x > 127
        cmp ball_vx, 127
        IF 0:  ; left <-
            mov 0, ball_x
            ; invert velocity
            mov 0, r1
            sub ball_vx, r1
            mov r1, ball_vx
        ENDIF
        
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
            ; invert velocity
            mov 0, r1
            sub ball_vx, r1
            mov r1, ball_vx
        ENDIF
    ENDIF



    ; y velocity
    add ball_vy, ball_y
    
    ; collision with top
    
    cmp ball_y, 127
    IF 0: ; ball_y > 127
        cmp ball_vy, 127
        IF 0:  ; up ^
            mov 0, ball_y
            ; invert velocity
            mov 0, r1
            sub ball_vy, r1
            mov r1, ball_vy
        ENDIF
        
    ENDIF

    ; collision with bottom

    mov ball_y, r0
    add ball_h, r0
    cmp r0, 127
    IF 0:
        cmp 127, ball_vy
        IF 0:  ; down V
            mov 127, ball_y
            sub ball_h, ball_y
            
            ; invert velocity
            mov 0, r1
            sub ball_vy, r1
            mov r1, ball_vy
        ENDIF
    ENDIF
    

    ret

; START
start:

; do some initialization of stuff
mov control1_h, control1_half
ror control1_half ; divide by 2
sub paddle_step, inverse_paddle_step


main:
    ; physics step
    jsr 7, physics_step



    ; render
    mov $00, color
    jsr 7, fill
    mov $FF, color
    mov ball_x, x
    mov ball_y, y
    mov ball_w, w
    mov ball_h, h
    jsr 7, draw_box

    mov control1_x, x
    mov control1_y, y
    mov control1_w, w
    mov control1_h, h
    jsr 7, draw_box
    
    rdw

    jmp 7, main

.align $7000 ; go to interrupt handler location

irq_handler:
    cmp interrupt_type, 1 ; keyboard interrupt
    IF 1: ; equal
        cmp [$9FFB], 0
        IF 1:
            ; keydown event
            cmp [$9FFC], 81 ; down arrow
            IF 1:
                mov paddle_step, control1_vy
            ENDIF
            
            cmp [$9FFC], 82 ; up arrow
            IF 1:
                mov inverse_paddle_step, control1_vy
            ENDIF
        ENDIF
        
        cmp [$9FFB], 0
        IF 0:
            ; keyup event
            cmp [$9FFC], 81 ; down arrow
            IF 1:
                cmp paddle_step, control1_vy
                IF 1:
                    mov 0, control1_vy
                ENDIF
            ENDIF

            cmp [$9FFC], 82 ; up arrow
            IF 1:
                cmp inverse_paddle_step, control1_vy
                IF 1:
                    mov 0, control1_vy
                ENDIF
            ENDIF
            nop
        ENDIF
    ENDIF
    ret



.align $9FFD ; go to interrupt address location
interrupt_type: .data u8, 0
interrupt_handler_location: .data u16, $7000