.globl setjmp
.text
setjmp:
	movq %rbx, (%rdi)
	movq %rcx, 8(%rdi)
	movq %rdx, 16(%rdi)
	pop  %rcx
	movq %rsp, 24(%rdi)
	movq %rbp, 32(%rdi)
	movq %rsi, 40(%rdi)
	movq %r12, 48(%rdi)
	movq %r13, 56(%rdi)
	movq %r14, 64(%rdi)
	movq %r15, 72(%rdi)
	movq %rcx, 80(%rdi)
	push %rcx
	movl $0, %eax
	ret

