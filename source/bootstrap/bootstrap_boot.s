.intel_syntax noprefix

# Stack Allocation
.section .bss
bss_start:
.align 0x1000
.global stack_bottom
stack_bottom:
.skip 16384 # 16 KiB Main Stack
.global stack_top
stack_top:
.global int_stack_bottom
int_stack_bottom:
.skip 16384 # 16 KiB Interrupt Stack
.global int_stack_top
int_stack_top:

.align 0x1000
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
.skip 0x8000

bss_end:

.code32
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
    push ecx
    push ebx # GRUB Multiboot2 info
    
    jmp enter_long

1:  hlt
    jmp 1b

enter_long:

    # Prepare bare minimum 64-bit paging

    # Clear tables
    lea edi, [PML4T]
    mov cr3, edi
    xor eax, eax
    mov ecx, 0x2c00
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
    add edi, 8
    add eax, 0x1000
    mov [edi], eax

    lea edi, [PT]
    mov ebx, 0x03
    mov ecx, 0x1000
    SetEntry:
        mov [edi], ebx
        add ebx, 0x1000
        add edi, 8
        loop SetEntry

    continue_entry:

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

    # Load TSS
    lea rax, [rip + TSS64]
    lea rdi, [rip + GDT_TSS]
    add rdi, 0x2

    mov [rdi], ax
    add rdi, 0x2
    shr rax, 16

    mov [rdi], al
    add rdi, 0x2
    shr rax, 8

    mov [rdi], al
    add rdi, 0x1
    shr rax, 8

    mov [rdi], eax

    mov ax, 0x28
    ltr ax

    # Send Initialization signal
    call send_init_serial

    # Fill in boot data location
    lea rax, [rip + data_start]
    mov [rip + multiboot_boot_info], rax
    lea rbx, [rip + data_end]
    sub rbx, rax
    mov [rip + multiboot_boot_info + 8], rbx

    # Process the multiboot tags
    pop rsi # GRUB Multiboot2 Structure
    lea rdi, [rip + multiboot_boot_info] # Target info structure
    call process_tags

    # Prepare the kernel module and get entry point
    lea rsi, [rip + multiboot_boot_info]
    lea rdi, [rip + kernel_entry_point]
    call prep_entry

    # Jump into the kernel
    lea rax, [rip + kernel_entry_point]
    mov rax, [rax]
    lea rdi, [rip + multiboot_boot_info] # Boot info for kernel
    jmp rax # Enter kernel at _start

.code32

.section .data

.global TSS64
TSS64:
    .4byte 0

    # RSP0
    .8byte stack_top

    .8byte 0
    .8byte 0
    .8byte 0

    # IST1
    .8byte int_stack_top

    .8byte 0
    .8byte 0
    .8byte 0
    .8byte 0
    .8byte 0
    .8byte 0
    .8byte 0

    .word 0
    .word 104
TSS64_end:

.global GDT64
.global GDT_Pointer
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
    .byte 0b00100000
    .byte 0

    # Data Segment
    GDT_Data:
    .word 0
    .word 0
    .byte 0
    .byte 0b10010010
    .byte 0b00000000
    .byte 0

    # User Code Segment
    GDT_User_Code:
    .word 0
    .word 0
    .byte 0
    .byte 0b11111010
    .byte 0b00100000
    .byte 0

    # User Data Segment
    GDT_User_Data:
    .word 0
    .word 0
    .byte 0
    .byte 0b11110010
    .byte 0b00000000
    .byte 0

    # TSS Segment
    # Address will have to be filled at runtime
    GDT_TSS:
    .word TSS64_end - TSS64
    .word 0
    .byte 0
    .byte 0x89
    .byte 0
    .byte 0
    .4byte 0
    .4byte 0

# GDT Pointer info
GDT_Pointer:
.word GDT_Pointer - GDT64 - 1
.4byte GDT64

.code64

kernel_entry_point:
    .long 0
    .long 0

multiboot_boot_info:
.8byte 0 # boot_start
.8byte 0 # boot_size
.8byte stack_bottom
.8byte int_stack_top
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
    lea edi, [thread_startup_pml4_address]
    mov edi, [edi]
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
.global thread_startup_pml4_address
thread_startup_pml4_address:
.4byte PML4T
