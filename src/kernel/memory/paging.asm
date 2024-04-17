section .text

global paging_enable
paging_enable:
    mov cr3, eax

    mov ebx, cr4        ; read current cr4
    or ebx, 0x10        ; set PSE
    mov cr4, ebx        ; update cr4

    mov ebx, cr0        ; read current cr0
    or ebx, 0x80000000  ; set PG
    mov cr0, ebx        ; update cr0
