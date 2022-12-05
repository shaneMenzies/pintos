OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

SECTIONS {

    . = 0xffffff0000000000;
    .text BLOCK(4K) : ALIGN(4K) {
        kernel_start = .;
        *(.init)
        text_start = .;
        *(.text)
        *(.fini)
    }

    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
    }
    kernel_end = .;
}