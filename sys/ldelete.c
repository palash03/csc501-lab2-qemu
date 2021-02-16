#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * sdelete  --  delete a semaphore by releasing its table entry
 *------------------------------------------------------------------------
 */

SYSCALL ldelete(int lockdescriptor)
{
	STATWORD ps;    
	int	pid;
	struct	lentry	*lptr = &locks[lockdescriptor];

	disable(ps);
	if (isbadlock(lockdescriptor) || lptr->lstate==LOCKFREE) {
		restore(ps);
		return(SYSERR);
	}
	lptr = &locks[lockdescriptor];
	lptr->lstate = DELETED;
	int i;
	if (nonempty(lptr->lhead)) 
    {
		struct pentry *p;
		while( (pid=getfirst(lptr->lhead)) != EMPTY)
        {
			p = &proctab[pid];
            p->lockwaitret = DELETED;
			//p->lockHeld[lockdescriptor] = 0;
            ready(pid,RESCHNO);
        }
		resched();
	}
	restore(ps);
	return(OK);
}
