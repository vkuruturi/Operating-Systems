.data
msg:
        .ascii "Hello world\n"		# string to be printed
        len = . - msg				# length of string
		
.text
		.global _start
_start:
		movq $len,%rdx				# string length
		movq $msg,%rsi				# message
		movq $500,%rax				# invalid syscall
		movq $1,%rbx				# 
		SYSCALL						

		movq $60,%rax				# sys_exit
		movq $0,%rdi				# with code 0
		SYSCALL
		