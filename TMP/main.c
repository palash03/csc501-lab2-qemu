/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>
#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }
/* <-------------------Test Case Semaphore-----------------------> */

void reader2 (char *a, int sem)
{
        kprintf ("  %s: to acquire lock\n", a);
        wait(sem);
        kprintf ("  %s: acquired lock\n", a);
        kprintf ("  %s: to release lock\n", a);
        signal(sem);
}

void writer2 (char *a, int sem)
{
        kprintf ("  %s: to acquire lock\n", a);
        wait(sem);
        kprintf ("  %s: acquired lock, sleep 10s\n", a);
        sleep (10);
        kprintf ("  %s: to release lock\n", a);
        signal(sem);
}

void test2 ()
{
        int     sem;
        int     sem1, sem2; //readers
        int     sem3; //writer

        kprintf("\nTest 3: test the semaphore implementation\n");
        sem  = screate (1);
        
        sem1 = create(reader2, 2000, 10, "Reader 1", 2, "A", sem);
        sem2 = create(reader2, 2000, 20, "Reader 2", 2, "B", sem);
        sem3 = create(writer2, 2000, 30, "Writer 1", 2, "C", sem);

        kprintf("-start writer C, then sleep 1s. lock granted to write (prio 20)\n");
        resume(sem3);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(sem1);
        sleep (1);
		
        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (sem2);
		sleep (3);
}

/*----------------------------------Locks test case---------------------------*/
void reader1 (char *a, int lck)
{
        kprintf ("  %s: to acquire lock\n", a);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", a);
        kprintf ("  %s: to release lock\n", a);
        releaseall (1, lck);
}

void writer1 (char *a, int lck)
{
        kprintf ("  %s: to acquire lock\n", a);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", a);
        sleep (10);
        kprintf ("  %s: to release lock\n", a);
        releaseall (1, lck);
}

void test1 ()
{
        int     lck;
        int     rd1, rd2; //readers
        int     wr1; //writer

        kprintf("\nTest 3: test the priority inheritence implementation\n");
        lck  = lcreate();
        
        rd1 = create(reader1, 2000, 25, "Reader 1", 2, "A", lck);
        rd2 = create(reader1, 2000, 30, "Reader 2", 2, "B", lck);
        wr1 = create(writer1, 2000, 20, "Writer 1", 2, "C", lck);

        kprintf("-start writer C, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
		
        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
		sleep (8);
}
/*----------------------------------Test 3---------------------------*/
void reader3 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer3 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test3 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 3 failed");

        rd1 = create(reader3, 2000, 25, "reader3", 2, "reader A", lck);
        rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck);
        wr1 = create(writer3, 2000, 20, "writer3", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
	assert (getprio(wr1) == 25, "Test 3 failed");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
	sleep (1);
	assert (getprio(wr1) == 30, "Test 3 failed");
	
	kprintf("-kill reader B, then sleep 1s\n");
	kill (rd2);
	sleep (1);
	assert (getprio(wr1) == 25, "Test 3 failed");

	kprintf("-kill reader A, then sleep 1s\n");
	kill (rd1);
	sleep(1);
	assert(getprio(wr1) == 20, "Test 3 failed");

        sleep (8);
        kprintf ("Test 3 OK\n");
}

int main()
{
		test1(); // Locks implementation
		test2(); // Semaphore implementation
		test3();
		shutdown();
}

