global pit_stub
extern pit_handler

pit_stub:
    cli
    pusha                   ; save EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10            ; data segment kernel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; REGISTERS pointer on stack
    mov eax, esp
    push eax

    call pit_handler

    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    sti
    iret
