#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>
struct lentry locks[NLOCKS];
    
void linit()
{
    int i,j;
    struct lentry *l;

    for(i=0;i<NLOCKS;i++)
    {
        l = &locks[i];
        l->lstate = LOCKFREE;
        l->ltail = 1 + (l->lhead = newqueue());
        l->check = 0;
        l->nr = 0;
        l->lprio = 0;
        struct pentry *p;
        for(j=0;j<NPROC;j++)
        {
            l->proc[j] = 0;
        }
    }
    nextlock = NLOCKS - 1;
}