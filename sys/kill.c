/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
extern void recCall(int,int);

SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev,i;
	extern int findMaxPrioProcess(int ldesc);

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;
					int i,j;
					struct lentry *lptr = &locks[pptr->lockid];
					
					for(j=0;j<NLOCKS;j++)
					{
						struct lentry *l = &locks[j];
						int temp = q[l->ltail].qprev;
						while(temp != l->lhead)
						{
							if(temp == pid)
							{
								for(i=0;i<NPROC;i++)
								{
									struct pentry *p = &proctab[i];
									if(p->lockHeld[j] == 1)
									{
										int x = pptr->lockid;
										dequeue(pid);
										lptr->lprio = findMaxPrioProcess(x);
										if(lptr->lprio > p->pprio)
										{
											p->pinh = lptr->lprio;
										}
										else
										{
											p->pinh = 0;
										}	
									}
								}
							}
							temp = q[temp].qprev;
						}
					}
					break;
					
	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
