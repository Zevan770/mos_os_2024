#include <asm/asm.h>

LEAF(msyscall)
	// Just use 'syscall' instruction and return.

	/* Exercise okk 4.1: Your code here. */
    syscall    
	jr		ra					# jump to $ra
END(msyscall)
