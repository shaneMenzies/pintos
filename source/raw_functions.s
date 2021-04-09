.intel_syntax noprefix

.section .data

.global fpu_status
.type fpu_status STT_OBJECT
fpu_status:
.word 1

idtr:
.space 8, 0

gdtr:
.space 8, 0

.section .text

.global return_ebx
.type return_ebx STT_FUNC
return_ebx:
    mov eax, ebx
    ret

.global set_gdt
.type set_gdt STT_FUNC
set_gdt:
    mov eax, [esp + 4]
    mov [gdtr + 2], eax
    mov ax, [esp + 8]
    mov [gdtr], ax
    lgdt gdtr
    ret

.global reload_gdt
.type reload_gdt STT_FUNC
reload_gdt:
    jmp 0x08:continue_reload
continue_reload:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

.global set_idt
.type set_idt STT_FUNC
set_idt:
    mov eax, [esp + 4]
    mov [idtr + 2], eax
    mov eax, [esp + 8]
    mov [idtr], ax
    lidt idtr
    ret

.global enable_interrupts
.type enable_interrupts STT_FUNC
enable_interrupts:
    sti
    ret

.global disable_interrupts
.type disable_interrupts STT_FUNC
disable_interrupts:
    cli
    ret

.global out_byte
.type out_byte STT_FUNC
out_byte:
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

.global in_byte
.type in_byte STT_FUNC
in_byte:
    mov dx, [esp + 4]
    in al, dx
    ret

.global out_word
.type out_word STT_FUNC
out_word:
    mov dx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret

.global in_word
.type in_word STT_FUNC
in_word:
    mov dx, [esp + 4]
    in ax, dx
    ret

.global out_dword
.type out_dword STT_FUNC
out_dword:
    mov dx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret

.global in_dword
.type in_dword STT_FUNC
in_dword:
    mov dx, [esp + 4]
    in eax, dx
    ret

.global io_wait
.type io_wait STT_FUNC
io_wait:
    mov al, 0
    out 0x80, al
    ret

.global test_soft_int
.type test_soft_int STT_FUNC
test_soft_int:
    int 33
    ret

.global fpu_init
.type fpu_init STT_FUNC
fpu_init:
    movw [fpu_status], 0xffff
    mov edx, cr0
    and edx, (-1) - ((1<<2) + (1<<3))
    mov cr0, edx
    fninit
    fnstsw [fpu_status]
    cmpw [fpu_status], 0
    jne .nofpu
    mov eax, 1
    ret

.nofpu:
    mov eax, 0
    ret
