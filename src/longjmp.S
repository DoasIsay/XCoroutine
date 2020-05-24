.globl longjmp
.text
longjmp:
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