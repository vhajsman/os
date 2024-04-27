%define VGA_SET_CURSOR_POS  0x0002
%define VGA_HIDE_CURSOR     0x0100
%define VGA_SHOW_CURSOR     0x0100
%define VGA_GET_CURSOR_POS

section .data
    cursor_row      db 0
    cursor_col      db 0
    cusror_start    db 0
    cursor_end      db 0

section .text
    global cursor_update
    global cursor_disable
    global cursor_enable
    global cursor_locate

    cursor_update:
        push bp
        mov bp, sp
        mov ax, VGA_SET_CURSOR_POS
        mov bh, 0x00

        mov dh, byte [bp + 4]
        mov dl, byte [bp + 5]
        mov dh, byte [cursor_row]
        mov dl, byte [cursor_col]

        int 0x10
        pop bp

        ret
    
    cursor_disable:
        mov ax, VGA_HIDE_CURSOR
        mov cx, 0x2000
        int 0x10

        ret
    
    cursor_enable:
        mov ax, VGA_SHOW_CURSOR
        mov cx, word [cusror_start]
        shl cx, 8
        mov cl, byte [cursor_end]
        or cx, cx
        int 0x10

        ret
    
    cursor_locate:
        mov ax, VGA_GET_CURSOR_POS
        mov bh, 0x00
        int 0x10

        mov [cursor_row], dh
        mov [cursor_col], dl

        ret