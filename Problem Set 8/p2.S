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
		#crashes because program tries to access memory that it does not have
		#permission to access, generating a SIGSEGV