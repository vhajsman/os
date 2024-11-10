MEMINFO         equ  1<<0
BOOTDEVICE      equ  1<<1
CMDLINE         equ  1<<2
MODULECOUNT     equ  1<<3
SYMT            equ  48 ; bits 4 & 5
MEMMAP          equ  1<<6
DRIVE           equ  1<<7
CONFIGT         equ  1<<8
BOOTLDNAME      equ  1<<9
APMT            equ  1<<10
VIDEO           equ  1<<11
VIDEO_FRAMEBUF  equ  1<<12
MAGIC_HEADER    equ  0x1BADB002

FLAGS           equ  MEMINFO | BOOTDEVICE | CMDLINE | MODULECOUNT | SYMT | MEMMAP | DRIVE | CONFIGT | BOOTLDNAME | VIDEO | VIDEO_FRAMEBUF
CHECKSUM        equ -(MAGIC_HEADER + FLAGS)

VM_BASE         equ 0xC0000000
PDE_INDEX       equ (VM_BASE >> 22)
PSE_BIT         equ 0x00000010
PG_BIT          equ 0x80000000

BOOTLOADER_MAGIC  equ  0x2BADB002

section .multiboot
    align 4

    dd MAGIC_HEADER
    dd FLAGS
    dd CHECKSUM

section .data
    align 4096
    global TEMP_PAGE_DIRECTORY

    TEMP_PAGE_DIRECTORY:
        dd 0x00000083
        times(PDE_INDEX - 1) dd 0

        dd 0x00000083
        times(1024 - PDE_INDEX - 1) dd 0 

section .initial_stack, nobits
    align 4

    stack_bottom:
        resb 104856
    stack_top:

section .text
    global _start

    global MAGIC_HEADER
    global BOOTLOADER_MAGIC

    ; mov ax, 0x0003
    ; int 0x10

    mov ax, 0x13
    int 0x10

    mov dx, 0x3D4
    mov al, 0x09
    out dx, al
    inc dx
    in al, dx
    and al, 0xE0
    or al, 0x00
    out dx, al

    _start:
        extern kmain
        extern kernel_exit
        
        mov esp, stack_top
        mov eax, BOOTLOADER_MAGIC

        push ebx
        push eax
        call kmain

        ; -- fallback --
        cli                 ; disable interrupts
        call kernel_exit    ; call fallback function

    loop:
        jmp loop
