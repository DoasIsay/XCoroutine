.globl save
.text
save:
#ifdef __i386__
    movl 4(%esp), %eax
    movl %ebx, (%eax)
    movl %ecx, 4(%eax)
    movl %edx, 8(%eax)
    popl %ebx
    movl %esp, 12(%eax)
    movl %ebp, 16(%eax)
    movl %esi, 20(%eax)
    movl %edi, 24(%eax)
    movl %ebx, 28(%eax)
    pushl %ebx
    movl (%eax), %ebx
    movl $0, %eax
    ret
#elif __x86_64__
    movq %rbx, (%rdi)
    movq %rcx, 8(%rdi)
    movq %rdx, 16(%rdi)
    pop  %rax
    movq %rsp, 24(%rdi)
    movq %rbp, 32(%rdi)
    movq %rsi, 40(%rdi)
    movq %r12, 48(%rdi)
    movq %r13, 56(%rdi)
    movq %r14, 64(%rdi)
    movq %r15, 72(%rdi)
    movq %rax, 80(%rdi)
    push %rax
    movl $0, %eax
    ret
#endif

.globl restore
.text
restore:
#ifdef __i386__
    movl 4(%esp), %eax
    movl (%eax), %ebx
    movl 4(%eax), %ecx
    movl 8(%eax), %edx
    movl 12(%eax), %esp
    movl 16(%eax), %ebp
    movl 20(%eax), %esi
    movl 24(%eax), %edi
    movl 28(%eax), %eax
    movl %eax, (%esp)
    movl $1, %eax
    ret
#elif __x86_64__
    movq (%rdi), %rbx
    movq 8(%rdi), %rcx
    movq 16(%rdi), %rdx
    movq 24(%rdi), %rsp
    movq 32(%rdi), %rbp
    movq 40(%rdi), %rsi
    movq 48(%rdi), %r12
    movq 56(%rdi), %r13
    movq 64(%rdi), %r14
    movq 72(%rdi), %r15
    movq 80(%rdi), %rax
    movq %rax, (%rsp)
    movl $1, %eax
    ret
#endif
