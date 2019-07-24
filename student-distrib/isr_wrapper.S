.data
	MIN_SYSCALL = 1;
	MAX_SYSCALL = 10;

.text
.globl   key_wrapper, rtc_wrapper, general_wrapper, master_wrapper, slave_wrapper, syscall_wrapper, pit_wrapper
.align   4
 
#Assembly wrapper for keyboard 
#Save all register and use iret
#instead of ret, as it is an interrupt
key_wrapper:
	pushfl
	cli
    pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esp
	pushl %ebp
	pushl %esi
	pushl %edi							#save all registers 
    cld
    call interrupt_keyboard				#call function
    popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax							#restore all registers
	popfl
    iret

#Assembly wrapper for rtc
#Save all register and use iret
#instead of ret, as it is an interrupt
rtc_wrapper:
	pushfl
	cli
    pushl %eax							#save all registers 
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esp
	pushl %ebp
	pushl %esi
	pushl %edi
    cld 
    call interrupt_rtc					#call function
    popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax			
	popfl #restore all registers
    iret
	
general_wrapper:
    iret
	
#Assembly wrapper for pit
#Save all register and use iret
#instead of ret, as it is an interrupt
pit_wrapper:
	pushfl
	cli
    pushl %eax							#save all registers 
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esp
	pushl %ebp
	pushl %esi
	pushl %edi
    cld 
    call interrupt_pit					#call function
    popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax							#restore all registers
	popfl
    iret

#Assembly wrapper for master
#Save all register and use iret
#instead of ret, as it is an interrupt	
master_wrapper:
    pushl %eax							#save all registers 
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esp
	pushl %ebp
	pushl %esi
	pushl %edi
    cld 
    call interrupt_master					#call function
    popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax							#restore all registers
    iret


#Assembly wrapper for slave
#Save all register and use iret
#instead of ret, as it is an interrupt	
slave_wrapper:
    pushl %eax							#save all registers 
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esp
	pushl %ebp
	pushl %esi
	pushl %edi
    cld 
    call interrupt_slave					#call function
    popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax							#restore all registers
    iret

syscall_wrapper:
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi							#save all registers 
	pushl %esp
	pushl %ebp
	cmpl $MIN_SYSCALL, %eax
	jl invalid_syscall					#validity check
	cmpl $MAX_SYSCALL, %eax
	jg invalid_syscall
	
	pushl %edx							#3rd arg
	pushl %ecx							#2nd arg	
	pushl %ebx							#1st arg
	
	call *jumptable(, %eax, 4)
	
	cmpl $-1, %eax
	addl $12, %esp			#pop all arguments off the stack
	je invalid_syscall
	
	popl %ebp
	popl %esp
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	iret
	
invalid_syscall:
	movl $-1, %eax
	popl %ebp
	popl %esp
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	iret
	
jumptable:
	.long 0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
	
.end
