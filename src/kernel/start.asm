;MBALIGN     equ  1<<0
;MEMINFO     equ  1<<1
;FLAGS       equ  MBALIGN | MEMINFO
;MAGIC       equ  0x1BADB002
;CHECKSUM    equ -(MAGIC + FLAGS)

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
FLAGS           equ  MEMINFO | BOOTDEVICE | CMDLINE | MODULECOUNT | SYMT | MEMMAP | DRIVE | CONFIGT | BOOTLDNAME | VIDEO_FRAMEBUF
MAGIC_HEADER    equ  0x1BADB002
CHECKSUM        equ -(MAGIC_HEADER + FLAGS)

BOOTLOADER_MAGIC  equ  0x2BADB002

section .multiboot
    align 4

    dd MAGIC_HEADER
    dd FLAGS
    dd CHECKSUM

section .data
    align 4096

section .initial_stack, nobits
    align 4

stack_bottom:
    resb 104856
stack_top:

section .text
    global _start

    global MAGIC_HEADER
    global BOOTLOADER_MAGIC

_start:
    ; extern paging_enable
    extern kmain
    extern kernel_exit

    ; Enable paging
    ; call paging_enable


    mov esp, stack_top
    mov eax, BOOTLOADER_MAGIC

    push ebx
    push eax
    call kmain

    call kernel_exit

    ; Prevent issuing interrupts in fallback.
    ; cli

; loop:
    ; jmp loop
