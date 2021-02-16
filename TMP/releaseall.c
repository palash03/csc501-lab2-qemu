#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

SYSCALL releaseall(int numlocks, int args, ...)
{
    STATWORD ps;
    disable(ps);
    int x = 0;
    struct pentry *p;
    unsigned long *a;
    //a = (unsigned long *)(&args) + (numlocks-1);
    a = (unsigned long *)(&args) + (numlocks-1);
    while(numlocks > 0)
    {
        int flag = release(*a--);
        if(flag == SYSERR)
        {
            x = 1;
        }
        numlocks--;
    }
    if(x == 1)
    {
        restore(ps);
        return SYSERR;
    }
    restore(ps);
    return OK;
}