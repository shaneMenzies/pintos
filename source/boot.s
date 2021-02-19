
# Multiboot header constants
.set ALIGN,      1<<0
.set MEMINFO,    1<<1
.set VIDEO,      1<<2
.set FLAGS,     (ALIGN | MEMINFO | VIDEO)
.set MAGIC,     0x1BADB002
.set CHECKSUM,  -(MAGIC + FLAGS)

.set VIDEO_MODE, 0
.set WIDTH,      1024
.set HEIGHT,     800
.set DEPTH,      24

# Multiboot header
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.skip 20
.long VIDEO_MODE
.long WIDTH
.long HEIGHT
.long DEPTH

# Stack Allocation
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# Entry point _start
.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp

    call kernel_main

    cli
1:  hlt
    jmp 1b

.size _start, . - _start
