#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

extern int ctr1000;
extern int findMaxPrioProcess(desc);

int isEmpty(int desc)
{
    struct lentry *l = &locks[desc];
    int temp = q[l->ltail].qprev;
    if(temp <= 0 || temp > NLOCKS)
    {
        return 0;
    }
    return 1;
}

void removeLast(int desc) 
{
    struct lentry *lptr = &locks[desc];
    int temp = q[lptr->ltail].qprev;   
    struct pentry *p = &proctab[temp];

    p->lockHeld[desc] = 1;
    if(p->pwaitmode == READ)
    {
        lptr->nr++;
    }
    lptr->type = p->pwaitmode;
    lptr->proc[currpid] = 0;
    lptr->proc[temp] = 1;
    p->pwaitmode = -1;
    p->pwaittime = 0;
    p->lockid = -1;
    ready(dequeue(temp),RESCHYES);
    lptr->lprio = findMaxPrioProcess(desc);
}

void removeAllReader(int desc)
{
    int s;
    struct lentry *lptr = &locks[desc];
    int temp = q[lptr->ltail].qprev;   
    while(temp != lptr->lhead)
    {
        struct pentry *p = &proctab[temp];
        if(p->pwaitmode == READ)
        {
            lptr->nr++;
            lptr->type = p->pwaitmode;
            lptr->proc[temp] = 1;
            lptr->proc[currpid] = 0;
            p->pwaitmode = -1;
            p->pwaittime = 0;
            p->lockid = -1;
            p->lockHeld[desc] = 1;
            s = q[temp].qprev;
            ready(dequeue(temp),RESCHNO);
            lptr->lprio = findMaxPrioProcess(desc);
        }
        else
        {
            break;
        }
        temp = s;
    }
    resched();
    return;
}

int checkWaitTime(int desc)
{
    struct lentry *lptr = &locks[desc];
    int temp = q[q[lptr->ltail].qprev].qprev;   
    struct pentry *p = &proctab[temp];
    if(p->pwaitmode == READ && temp == q[lptr->ltail].qprev)
    {
        return 1;
    }    
    return 0;
}

void assignWaitTime(int desc)
{   
    int c = 0;
    struct lentry *l = &locks[desc];
    int temp = q[l->ltail].qprev;
    while(temp != l->lhead)
    {
        struct pentry *p = &proctab[temp];
        if(p->pwaitmode == WRITE && c == 0)
        {
            p->tempwaittime = ctr1000 - p->pwaittime;
            c++;
        }
        else if(p->pwaitmode == WRITE && c > 0)
        {
            break;
        }
        else
        {
            p->tempwaittime = ctr1000 - p->pwaittime;
        }
        //kprintf("Time: %d\n",p->tempwaittime);
        temp = q[temp].qprev;
    }
}

SYSCALL release(int desc)
{
    STATWORD ps;    
	disable(ps);
    if(isbadlock(desc))
    {
        restore(ps);
        return SYSERR;
    }
    struct pentry *p = &proctab[currpid];
    struct lentry *lptr = &locks[desc];
    if(p->lockHeld[desc] != 1)
    {
        restore(ps);
        return SYSERR;
    }
    //kprintf("READ/WRITE: %d\n",lptr->type);
    p->lockHeld[desc] = 0;
    if(lptr->type == READ)
    {
        lptr->nr --;
        if(lptr->nr == 0)
        {
            if(isEmpty(desc) == 0)
            {
                lptr->check = 0;
                restore(ps);
                return(OK);
            }
            removeLast(desc);     
        }
        restore(ps);
        return(OK);
    }
    // Lock is acquired in Write Mode
    int temp = q[lptr->ltail].qprev;
    if(temp <= 0 || temp > NPROC)
    {
        restore(ps);
        return(SYSERR);
    }
    struct pentry *ptr = &proctab[temp];
    lptr->proc[currpid] = 0;
    if(ptr->pwaitmode == READ)
    {
        removeAllReader(desc);
    }   
    else
    {
        int x = checkWaitTime(desc);
        int s;
        if(x == 1) //Queue consists of Writer in the end followed by a reader
        {
            struct pentry *p2 = &proctab[temp];
            int t2 = q[temp].qprev;
            assignWaitTime(desc);
            while(t2 != lptr->lhead)
            {
                struct pentry *p3 = &proctab[t2];
                if(p3->pwaitmode == READ && (p3->tempwaittime - p2->tempwaittime) < 400)
                {
                    lptr->nr++;
                    lptr->type = p3->pwaitmode;
                    lptr->proc[t2] = 1;
                    lptr->proc[currpid] = 0;
                    p3->pwaitmode = -1;
                    p3->pwaittime = 0;
                    p3->lockid = -1;
                    p3->lockHeld[desc] = 1;
                    s = q[t2].qprev;
                    ready(dequeue(t2),RESCHNO);
                    lptr->lprio = findMaxPrioProcess(desc);
                }
                else
                {
                    break;
                }
                t2 = s;
            }
            resched();
        }
        else
        {
            removeLast(desc);   
        }
    }
    restore(ps);
    return(OK);
}