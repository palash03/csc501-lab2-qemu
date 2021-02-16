#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

extern int isEmpty(int);

void recCall(int ldesc, int pid)
{
    struct lentry *l = &locks[ldesc];
    int i,j;
    struct pentry *pt = &proctab[pid];
    if(l->lprio > pt->pprio)
    {
        pt->pinh = l->lprio;
        //kprintf("Setting pinh for pid: %d -> %d\n",pid,pt->pinh);
        for(i=0;i<NLOCKS;i++)
        {
            struct lentry *lptr = &locks[i];
            int temp = q[lptr->ltail].qprev;
            while(temp != lptr->lhead)
            {
                if(temp == pid)
                {
                    for(j=0;j<NPROC;j++)
                    {
                        struct pentry *p = &proctab[j];
                        if(p->lockHeld[i] == 1)
                        {
                            recCall(lptr->lockid,j);
                        }
                    }
                }
                temp = q[temp].qprev;
            }
        }
    }
}

int findMaxPrioProcess(int ldesc)
{
    int i,maxPrio=0;
    struct lentry *l = &locks[ldesc];
    int temp = q[l->ltail].qprev;
    struct pentry *p; 
    while(temp != l->lhead)
    {
        p = &proctab[temp];
        maxPrio = max(maxPrio,p->pprio);
        temp = q[temp].qprev;
    }
    return maxPrio;
}

void insertToQueue(int ldesc, int priority, int type)
{
    extern int ctr1000;
    struct lentry *l = &locks[ldesc];
    insert(currpid,l->lhead,priority);
    struct pentry *p = &proctab[currpid];
    l->lprio = max(p->pprio,l->lprio);
    p->pwaitmode = type;
    p->pwaittime = ctr1000;
    p->pstate = PRWAIT;
    p->lockid = ldesc;
    p->lockwaitret = OK;
}

SYSCALL lock(int ldesc1, int type, int priority)
{
    STATWORD ps;    
    disable(ps);
    extern int ctr1000;
    struct pentry *p = &proctab[currpid];
    struct lentry *lptr;
    if (isbadlock(ldesc1) || (lptr= &locks[ldesc1])->lstate==LOCKFREE || lptr->lstate == DELETED) {
		restore(ps);
		return(SYSERR);
	}
    struct lentry *l = &locks[ldesc1];
    if(p->lockwaitret == DELETED)
    {
        restore(ps);
		return(SYSERR);
    }       
    if(l->check == 0)  /*  If lock is not acquired  */
    {
        struct pentry *ptr = &proctab[currpid];
        if(type == READ)
        {
            l->nr++;
        }
        l->check = 1;
        l->type = type;
        ptr->lockHeld[ldesc1] = 1;
        l->proc[currpid] = 1;
    }
    else if(l->check == 1) /*   if lock is already acquired     */
    {
        if(isEmpty(ldesc1) == 0) //when queue is empty
        {
            if(l->type == READ && type == READ && priority > lastkey(l->ltail))
            {
                l->nr++;
                struct pentry *ptr = &proctab[currpid];
                ptr->lockHeld[ldesc1] = 1;
                l->proc[currpid] = 1;                         
                restore(ps);
                return OK;
            }
        }        
        if(p->pprio > findMaxPrioProcess(ldesc1))
        {
            int i,sum = 0;
            l->lockid = ldesc1;
            insertToQueue(ldesc1,priority,type);
            for(i=0;i<NPROC;i++)
            {
                if(l->proc[i] == 1)
                {
                    recCall(ldesc1,i);
                }
            }
            resched();  
            restore(ps);
            return p->lockwaitret;
        }
        if(p->pprio <= findMaxPrioProcess(ldesc1))
        {
            if(type == READ && l->type == READ)
            {
                if(priority > lastkey(l->ltail))
                {
                    l->nr++;
                    struct pentry *ptr = &proctab[currpid];
                    ptr->lockHeld[ldesc1] = 1;
                    l->proc[currpid] = 1;                         
                    restore(ps);
                    return OK;
                }
                else
                {
                    struct lentry *lptr = &locks[ldesc1];
                    lptr->lockid = ldesc1;    
                    insertToQueue(ldesc1,priority,type);
                    resched();
                    restore(ps);
                    return p->lockwaitret;
                }            
            }
            else
            {
                struct lentry *lptr = &locks[ldesc1];
                lptr->lockid = ldesc1;
                insertToQueue(ldesc1,priority,type);
                resched();
                restore(ps);
                return p->lockwaitret;   
            }
        }
    }
    restore(ps);
    return OK;
}
