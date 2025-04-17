	.file	"base_main.c"
	.text
	.globl	argp_program_version
	.section	.rodata
.LC0:
	.string	"1.0"
	.section	.data.rel.local,"aw"
	.align 8
	.type	argp_program_version, @object
	.size	argp_program_version, 8
argp_program_version:
	.quad	.LC0
	.globl	argp_program_bug_address
	.section	.rodata
.LC1:
	.string	"<your@email.com>"
	.section	.data.rel.local
	.align 8
	.type	argp_program_bug_address, @object
	.size	argp_program_bug_address, 8
argp_program_bug_address:
	.quad	.LC1
	.data
	.align 32
	.type	doc, @object
	.size	doc, 60
doc:
	.string	"Implemention base64/base32 encode and decode functions on C"
	.align 8
	.type	args_doc, @object
	.size	args_doc, 14
args_doc:
	.string	"[FILENAME]..."
	.section	.rodata
.LC2:
	.string	"verbose"
.LC3:
	.string	"Produce verbose output"
.LC4:
	.string	"output"
.LC5:
	.string	"FILE"
	.align 8
.LC6:
	.string	"Output to FILE instead of stdout"
.LC7:
	.string	"encode"
.LC8:
	.string	"Encode input (default)"
.LC9:
	.string	"decode"
.LC10:
	.string	"Decode input"
.LC11:
	.string	"base64"
.LC12:
	.string	"Base64 encoding (default)"
.LC13:
	.string	"base32"
.LC14:
	.string	"Base32 encoding"
	.section	.data.rel.local
	.align 32
	.type	options, @object
	.size	options, 336
options:
	.quad	.LC2
	.long	118
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	.LC3
	.zero	8
	.quad	.LC4
	.long	111
	.zero	4
	.quad	.LC5
	.long	0
	.zero	4
	.quad	.LC6
	.zero	8
	.quad	.LC7
	.long	101
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	.LC8
	.zero	8
	.quad	.LC9
	.long	100
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	.LC10
	.zero	8
	.quad	.LC11
	.long	52
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	.LC12
	.zero	8
	.quad	.LC13
	.long	50
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	.LC14
	.zero	8
	.quad	0
	.zero	40
	.text
	.type	parse_opt, @function
parse_opt:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movl	%edi, -20(%rbp)
	movq	%rsi, -32(%rbp)
	movq	%rdx, -40(%rbp)
	movq	-40(%rbp), %rax
	movq	40(%rax), %rax
	movq	%rax, -8(%rbp)
	cmpl	$16777217, -20(%rbp)
	je	.L13
	cmpl	$16777217, -20(%rbp)
	jg	.L3
	cmpl	$118, -20(%rbp)
	je	.L4
	cmpl	$118, -20(%rbp)
	jg	.L3
	cmpl	$111, -20(%rbp)
	je	.L5
	cmpl	$111, -20(%rbp)
	jg	.L3
	cmpl	$101, -20(%rbp)
	je	.L6
	cmpl	$101, -20(%rbp)
	jg	.L3
	cmpl	$100, -20(%rbp)
	je	.L7
	cmpl	$100, -20(%rbp)
	jg	.L3
	cmpl	$52, -20(%rbp)
	je	.L8
	cmpl	$52, -20(%rbp)
	jg	.L3
	cmpl	$0, -20(%rbp)
	je	.L9
	cmpl	$50, -20(%rbp)
	je	.L10
	jmp	.L3
.L4:
	movq	-8(%rbp), %rax
	movl	$1, 8(%rax)
	jmp	.L11
.L5:
	movq	-8(%rbp), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, (%rax)
	jmp	.L11
.L6:
	movq	-8(%rbp), %rax
	movl	$1, 12(%rax)
	jmp	.L11
.L7:
	movq	-8(%rbp), %rax
	movl	$0, 12(%rax)
	jmp	.L11
.L8:
	movq	-8(%rbp), %rax
	movl	$1, 16(%rax)
	jmp	.L11
.L10:
	movq	-8(%rbp), %rax
	movl	$0, 16(%rax)
	jmp	.L11
.L9:
	movq	-8(%rbp), %rax
	movl	32(%rax), %eax
	addl	$1, %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-8(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	realloc@PLT
	movq	-8(%rbp), %rdx
	movq	%rax, 24(%rdx)
	movq	-8(%rbp), %rax
	movq	24(%rax), %rsi
	movq	-8(%rbp), %rax
	movl	32(%rax), %eax
	leal	1(%rax), %ecx
	movq	-8(%rbp), %rdx
	movl	%ecx, 32(%rdx)
	cltq
	salq	$3, %rax
	leaq	(%rsi,%rax), %rdx
	movq	-32(%rbp), %rax
	movq	%rax, (%rdx)
	jmp	.L11
.L3:
	movl	$7, %eax
	jmp	.L12
.L13:
	nop
.L11:
	movl	$0, %eax
.L12:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	parse_opt, .-parse_opt
	.section	.data.rel.local
	.align 32
	.type	argp, @object
	.size	argp, 56
argp:
	.quad	options
	.quad	parse_opt
	.quad	args_doc
	.quad	doc
	.zero	24
	.section	.rodata
.LC15:
	.string	"-"
.LC16:
	.string	"Verbose output ENABLED"
.LC17:
	.string	"unable to allocate buffer"
.LC18:
	.string	"Type: "
.LC19:
	.string	"%zu chars were read\n"
.LC20:
	.string	"you typed: %s \n"
	.align 8
.LC21:
	.string	"Running in %s mode (encoding: %s)\n"
.LC22:
	.string	"\nENCODED: "
.LC23:
	.string	"\nDECODED: "
	.text
	.globl	main
	.type	main, @function
main:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	addq	$-128, %rsp
	movl	%edi, -116(%rbp)
	movq	%rsi, -128(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	leaq	.LC15(%rip), %rax
	movq	%rax, -48(%rbp)
	movl	$0, -40(%rbp)
	movl	$1, -36(%rbp)
	movl	$1, -32(%rbp)
	movq	$0, -24(%rbp)
	movl	$0, -16(%rbp)
	leaq	-48(%rbp), %rcx
	movq	-128(%rbp), %rdx
	movl	-116(%rbp), %eax
	movq	%rcx, %r9
	movl	$0, %r8d
	movl	$0, %ecx
	movl	%eax, %esi
	leaq	argp(%rip), %rax
	movq	%rax, %rdi
	call	argp_parse@PLT
	movl	-40(%rbp), %eax
	testl	%eax, %eax
	setne	%al
	movb	%al, -105(%rbp)
	cmpb	$0, -105(%rbp)
	je	.L15
	leaq	.LC16(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
.L15:
	movq	$32, -96(%rbp)
	movq	-96(%rbp), %rax
	movq	%rax, %rdi
	call	malloc@PLT
	movq	%rax, -104(%rbp)
	movq	-104(%rbp), %rax
	testq	%rax, %rax
	jne	.L16
	leaq	.LC17(%rip), %rax
	movq	%rax, %rdi
	call	perror@PLT
	movl	$1, %edi
	call	exit@PLT
.L16:
	cmpb	$0, -105(%rbp)
	je	.L17
	leaq	.LC18(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.L17:
	movq	stdin(%rip), %rdx
	leaq	-96(%rbp), %rcx
	leaq	-104(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	getline@PLT
	movq	%rax, -88(%rbp)
	cmpb	$0, -105(%rbp)
	je	.L18
	movq	-88(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC19(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.L18:
	cmpb	$0, -105(%rbp)
	je	.L19
	movq	-104(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC20(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.L19:
	cmpb	$0, -105(%rbp)
	je	.L20
	movl	-32(%rbp), %eax
	testl	%eax, %eax
	je	.L21
	leaq	.LC11(%rip), %rdx
	jmp	.L22
.L21:
	leaq	.LC13(%rip), %rdx
.L22:
	movl	-36(%rbp), %eax
	testl	%eax, %eax
	je	.L23
	leaq	.LC7(%rip), %rax
	jmp	.L24
.L23:
	leaq	.LC9(%rip), %rax
.L24:
	movq	%rax, %rsi
	leaq	.LC21(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.L20:
	movl	-36(%rbp), %eax
	testl	%eax, %eax
	je	.L25
	movl	-32(%rbp), %eax
	testl	%eax, %eax
	je	.L25
	movl	-40(%rbp), %eax
	testl	%eax, %eax
	setne	%al
	movzbl	%al, %edx
	movq	-104(%rbp), %rax
	movq	-88(%rbp), %rcx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	base64_encode@PLT
	movq	%rax, -80(%rbp)
	cmpb	$0, -105(%rbp)
	je	.L26
	leaq	.LC22(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.L26:
	movq	-80(%rbp), %rax
	movq	%rax, %rdi
	call	puts@PLT
	movq	-80(%rbp), %rax
	movq	%rax, %rdi
	call	free@PLT
	jmp	.L27
.L25:
	movl	-36(%rbp), %eax
	testl	%eax, %eax
	jne	.L28
	movl	-32(%rbp), %eax
	testl	%eax, %eax
	je	.L28
	movl	-40(%rbp), %eax
	testl	%eax, %eax
	setne	%al
	movzbl	%al, %edx
	movq	-104(%rbp), %rax
	movq	-88(%rbp), %rcx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	base64_decode@PLT
	movq	%rax, -72(%rbp)
	cmpb	$0, -105(%rbp)
	je	.L29
	leaq	.LC23(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.L29:
	movq	-72(%rbp), %rax
	movq	%rax, %rdi
	call	puts@PLT
	movq	-72(%rbp), %rax
	movq	%rax, %rdi
	call	free@PLT
	jmp	.L27
.L28:
	movl	-36(%rbp), %eax
	testl	%eax, %eax
	je	.L30
	movl	-32(%rbp), %eax
	testl	%eax, %eax
	jne	.L30
	movl	-40(%rbp), %eax
	testl	%eax, %eax
	setne	%al
	movzbl	%al, %edx
	movq	-104(%rbp), %rax
	movq	-88(%rbp), %rcx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	base32_encode@PLT
	movq	%rax, -64(%rbp)
	cmpb	$0, -105(%rbp)
	je	.L31
	leaq	.LC22(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.L31:
	movq	-64(%rbp), %rax
	movq	%rax, %rdi
	call	puts@PLT
	movq	-64(%rbp), %rax
	movq	%rax, %rdi
	call	free@PLT
	jmp	.L27
.L30:
	movl	-36(%rbp), %eax
	testl	%eax, %eax
	jne	.L27
	movl	-32(%rbp), %eax
	testl	%eax, %eax
	jne	.L27
	movl	-40(%rbp), %eax
	testl	%eax, %eax
	setne	%al
	movzbl	%al, %edx
	movq	-104(%rbp), %rax
	movq	-88(%rbp), %rcx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	base32_decode@PLT
	movq	%rax, -56(%rbp)
	cmpb	$0, -105(%rbp)
	je	.L32
	leaq	.LC23(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.L32:
	movq	-56(%rbp), %rax
	movq	%rax, %rdi
	call	puts@PLT
	movq	-56(%rbp), %rax
	movq	%rax, %rdi
	call	free@PLT
.L27:
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	free@PLT
	movl	$0, %eax
	movq	-8(%rbp), %rdx
	subq	%fs:40, %rdx
	je	.L34
	call	__stack_chk_fail@PLT
.L34:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	main, .-main
	.ident	"GCC: (GNU) 14.2.1 20250207"
	.section	.note.GNU-stack,"",@progbits
