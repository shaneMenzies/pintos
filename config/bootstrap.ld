
ENTRY(_start)

SECTIONS {

    .text BLOCK(4K) : ALIGN(4K) {
        . = ALIGN(8);
        KEEP(*(.multiboot))
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K) {
        data_start = .;
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
        data_end = .;
    }
}