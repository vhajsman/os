section .text
    global gdt_load

gdt_load:
    mov eax, [esp + 4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    cli
    mov	eax, cr0
    or eax, 1
    mov	cr0, eax

    jmp 0x08:far_jump
far_jump:
    ret