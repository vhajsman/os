ENTRY(_start)

MEMORY
{
    RAM (rxw) : ORIGIN = 0x00100000, LENGTH = 32M
}

SECTIONS
{
    __kernel_section_start = .;
    /* text section */
    .multiboot : {
        /* set multiboot header */
        LONG(0x1BADB002)
        LONG(0x00000003)
        LONG(0xE4524FFB)
    } > RAM
    
    .text : {
        __kernel_text_section_start = .;
        . = ALIGN(4);
        
        code = .; _code = .; __code = .;
        *(.text)
        *(.rodata)
        . = ALIGN(4096);
        __kernel_text_section_end = .;
    } > RAM

    .middlestack : {
        __kernel_middlestack_section_start = .;
        middlestack = .; _middlestack = .; __middlestack = .;
        *(.middlestack)
        . = ALIGN(4096);
        __kernel_middlestack_section_end = .;
    } > RAM

    /* initialized data section */
    .data : {
        __kernel_data_section_start = .;
        data = .; _data = .; __data = .;
        *(.data)
        . = ALIGN(4096);
        __kernel_data_section_end = .;
    } > RAM

    /* constant data section */
    .rodata : {
        __kernel_rodata_section_start = .;
        *(.rodata)
        __kernel_rodata_section_end = .;
    } > RAM

    /* un-initialized data section */
    .bss : {
        __kernel_bss_section_start = .;

        bootstrap_stack = .;
        . = . + 0x1000;

        bss = .; _bss = .; __bss = .;
        *(.bss)
        . = ALIGN(4096);
        __kernel_bss_section_end = .;
    } > RAM

    .dma_buffers : {
        __dma_buffers_start = .;
        . = ALIGN(4096);
        KEEP(*(.dma_buffers))
        . = . + 0x4000;
        . = ALIGN(4096);
        __dma_buffers_end = .;
    } > RAM

    end = .; 
    _end = .; 
    __end = .;
    
    __kernel_section_end = .;

    .note.GNU-stack : {
        KEEP(*(.note.GNU-stack))
    }
}
