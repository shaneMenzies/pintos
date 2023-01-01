; _start - entry point for user-space programs
;   called by loader, sets up before calling constructors and main

.section .text

.global _start
_start:
    ; Called with argc and argv in rdi and rsi
    pushq %rsi
    pushq %rdi

    ; Run global constructors
    call _init

    ; Pop argc and argv back
    popq %rdi
    popq %rsi

    ; Run main
    call main

    ; Run destructors
    call _fini

    ; Return will call end of process task
    ret
.size _start, . - _start
