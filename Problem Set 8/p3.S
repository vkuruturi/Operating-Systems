.data
msg:
        .ascii "Hello world\n"		# string to be printed
        len = . - msg				# length of string
		
.text
		.global _start
_start:
		movq $len,%rdx				# string length
		movq $msg,%rsi				# message
		movq $1,%rax				# sycall# for write
		movq $1,%rbx				# write to stdout
		SYSCALL						

		movq $60,%rax				# sys_exit
		movq $1,%rdi				# with code 1
		SYSCALL
		