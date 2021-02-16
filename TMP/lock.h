#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef	NLOCKS
#define	NLOCKS		50	/* number of locks, if not defined	*/
#endif

#define	LOCKFREE	'\01'		/* this lock is free		*/
#define	LOCKUSED	'\02'		/* this lock is used		*/

#define READ 1
#define WRITE 2

void linit();
SYSCALL lcreate(void);
SYSCALL lock(int,int,int);
SYSCALL ldelete(int);
SYSCALL release(int);
SYSCALL releaseall(int,int, ...);

struct	lentry	{		/* locks table entry		*/
	char	lstate;		/* the state LOCKFREE or LOCKUSED or LOCKTYPE		*/
    int lockid;
    int check;
    int	lprio;		/* maximum priority among all the processes waiting in the lock’s wait queue    */
	int	lhead;		/* q index of head of list		*/
	int	ltail;		/* q index of tail of list		*/
    int type;       // Read or Write
    int proc[NPROC];
    int nr;         // Number of readers
};

extern	struct	lentry	locks[];
extern	int	nextlock;

#define	isbadlock(s)	(s<0 || s>=NLOCKS)

#endif
