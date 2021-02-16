#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newlock();

SYSCALL lcreate()
{
	STATWORD ps;    
	int	ldesc;

	disable(ps);
	if ((ldesc=newlock())==SYSERR ) {
		restore(ps);
		return(SYSERR);
	}
	restore(ps);
	return(ldesc);
}

LOCAL int newlock()
{
	int	desc;
	int	i;

	for (i=0 ; i<NLOCKS ; i++) {
		desc=nextlock--;
		if (nextlock < 0)
			nextlock = NLOCKS-1;
		if (locks[desc].lstate==LOCKFREE) {
			locks[desc].lstate = LOCKUSED;
			//locks[desc].ldesc = desc; //new added
			return(desc);
		}
	}
	return(SYSERR);
}
