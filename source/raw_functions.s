.section .text

.global return_ebx
.type return_ebx, @function
return_ebx:
    movl %ebx, %eax
    ret
