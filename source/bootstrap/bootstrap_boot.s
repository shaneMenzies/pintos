.intel_syntax noprefix

# Stack Allocation
.section .bss
bss_start:
.align 16
.global stack_bottom
stack_bottom:
.skip 32768 # 32 KiB Stack
.global stack_top
stack_top:

.align 16
.global PML4T
.global PDPT
.global PDT
.global PT
PML4T:
.skip 0x1000
PDPT:
.skip 0x1000
PDT:
.skip 0x1000
PT:
.skip 0x2000
bss_end:

# Entry point _start
.section .text
.global _start
.type _start, @function
_start:
    cli
    lea esp, [stack_top]
    lea ebp, [stack_top]

    # Prepare stack for future 64-bit use
    xor ecx, ecx
    lea eax, [multiboot_boot_info]
    push ecx
    push eax
    lea eax, [GDT64]
    push ecx
    push eax
    lea eax, [kernel_entry_point]
    push ecx
    push eax
    mov edx, ebx

    # Process the multiboot tags
    push ebx
    lea eax, [multiboot_boot_info]
    push eax
    lea eax, [data_start]
    mov [multiboot_boot_info], eax
    lea ebx, [data_end]
    sub ebx, eax
    mov [multiboot_boot_info + 8], ebx
    call process_tags
    add esp, 8

    # Prepare the kernel module and get entry point
    lea eax, [multiboot_boot_info]
    push eax
    lea eax, [kernel_entry_point]
    push eax
    call prep_64
    add esp, 8

    call send_init_serial

    # Confirm that a valid entry point was given
    mov eax, [kernel_entry_point]
    test eax, eax
    jnz enter_long
    mov eax, [kernel_entry_point + 4]
    test eax, eax
    jnz enter_long

1:  hlt
    jmp 1b

enter_long:

    # Prepare bare minimum 64-bit paging

    # Clear tables
    lea edi, [PML4T]
    mov cr3, edi
    xor eax, eax
    mov ecx, 4096
    rep stosd
    mov edi, cr3

    lea eax, [PDPT]
    add eax, 3
    mov [edi], eax
    add edi, 0x1000
    add eax, 0x1000
    mov [edi], eax
    add edi, 0x1000
    add eax, 0x1000
    mov [edi], eax
    add edi, 8
    add eax, 0x1000
    mov [edi], eax
    add edi, (0x1000 - 8)

    mov ebx, 0x03
    mov ecx, 1024
    SetEntry:
        mov [edi], ebx
        add ebx, 0x1000
        add edi, 8
        loop SetEntry

    # Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    # Switch into Compatibility mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    # Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    # Jump into 64-bit code
    lea eax, [GDT_Pointer]
    lgdt [eax]
    jmp 0x08:long_jump

long_jump:
.code64

    # Reload gdt segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    # Jump into the kernel
    pop rax
    mov rax, [rax]
    pop rsi
    pop rdi
    jmp rax

.code32

.section .data

kernel_entry_point:
    .long 0
    .long 0

.global GDT64
.global GDT_pointer
GDT64:

    # Null Segment
    GDT_Null:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0
    .byte 1
    .byte 0

    # Code Segment
    GDT_Code:
    .word 0
    .word 0
    .byte 0
    .byte 0b10011010
    .byte 0b10101111
    .byte 0

    # Data Segment
    GDT_Data:
    .word 0
    .word 0
    .byte 0
    .byte 0b10010010
    .byte 0b00000000
    .byte 0

# GDT Pointer info
GDT_Pointer:
.word GDT_Pointer - GDT64 - 1
.4byte GDT64

.global multiboot_boot_info
multiboot_boot_info:
.4byte 0 # boot_start
.4byte 0
.4byte 0 # boot_size
.4byte 0
.4byte bss_start
.4byte 0
.4byte bss_end
.4byte 0
.skip 386

# New thread entry point
.section .text
.global thread_startup
.type thread_startup, @function
.align 0x1000
.code16
thread_startup:
    cli
    cld

    # Set the paging table
    mov edi, 0xd000 
    mov cr3, edi

    # Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    # Enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31 | 1
    mov cr0, eax

    lea eax, [GDT_Pointer]
    lgdt [eax]

    # Reload gdt segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ds, ax

    # Save address of the target code
    lea ebx, [thread_startup_target_code]

    # Jump into 64-bit code
    jmp 0x08:thread_long_jump

.global thread_long_jump
thread_long_jump:
.code64

    # Setup stack
    mov rsp, [rbx + 8]
    mov rbp, [rbx + 8]

    # Jump into the kernel
    mov rax, [rbx]
    jmp rax

.global thread_startup_end
thread_startup_end:

.section .data

.global thread_startup_target_code
thread_startup_target_code:
.8byte 0
.global next_thread_stack_top
next_thread_stack_top:
.8byte 0
