//this code is my own work written without an outside tutor or code by other students - Ana Noriega
//based on signals from class

//On signals.c
/* Program to illustrate the use of POSIX signals on UNIX
		The program runs a computational loop to
		compute perfect numbers starting at a fixed
		point.

	A time alarm signal is used to periodically print status
	An interrupt signal is used for status on demand
	A quit signal is used to reset the test interval (or terminate) */

//the goal of compute will be:
/*
compute perfect numbers starting at a fixed point.
multiple computes can run, so instead of reading command line- manage will feed this number? NO, THIS IS NOT FORK. LAUNCHED FROM TERMINAL
Compute processes are responsible for updating the bit map, as well as their own process statistics.
Compute must send Manage any perfect # found.
Compute processes that hit the end should wrap around but STOP AT THEIR STRATING POINT AND EXEC "report -k"
All process should terminate cleanly on INTR, QUIT, and HANGUP signals.
As in delete their process entry from shared memory and terminate.
*/

#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "defs.h"
#define KEY (key_t)88582	/*key for shared memory segment */

int whatInd(int pid, struct mesg_buffer msg);
void perfect(int start, struct mesg_buffer);
int tested( int A[],  int k );
void set( int A[],  int k );
void terminate();

//sigjmp_buf jmpenv; /* environment saved by setjmp*/

int n; /* global variable indicating current test point */

//they keep process statistics and write to shared memory, multiple Computes can run
struct shMem *bitmap;
int proInd;
int sid;//shared mem
int qid; //message queue
//Compute must send Manage any perfect # found.
//int messId;///BUG



int main(int argc, char* argv[]) {


    struct mesg_buffer mesg;
    int begin; /* starting point for next search*/
    /* interrupt routines*/
    //void status();
    //void query();

    //sigset_t mask;
    //struct sigaction action;

    if (argc > 1 ) {begin = atoi(argv[1]);
    } else begin = 2;
//	if (sigsetjmp(jmpenv,0)) {
//		printf("Enter search starting point (0 to terminate): ");
//		scanf("%d",&begin);
//		if (begin==0) exit(0);
//		sigprocmask(SIG_UNBLOCK, &mask, NULL);
//		}
//	else begin=2;


    /* create queue if necessary */
    if ((qid=msgget(KEY,IPC_CREAT |0660))== -1) {
        perror("msgget");
        exit(1);
    }
    /* create shared segment if necessary */
    if ((sid=shmget(KEY,sizeof (struct shMem),IPC_CREAT |0660))== -1) {
        perror("shmget");
        exit(1);
    }
    /* map it into our address space*/

    if ((bitmap = ((struct shMem *)shmat(sid, 0, 0)))== (struct shMem *) -1) {
        perror("shmat");
        exit(2);
    }


    //find out who you are in the process array of shmem by asking manage
    mesg.mesg_type = 1; //diffrent types get difrent fxns
    //find out what process you are
    int pid = getpid();
    //find out what index Manage gave you for the shmem
    proInd = whatInd(pid, mesg);
    //the shmem can only fit 20 processes
    if(proInd > 19) {
        if (shmdt((char  *) bitmap) == -1) {
            perror("shmdt");
            exit(-1);
        }
        terminate();
    }
    /* Status Routine will handle timer and INTR */
    //there is only one manage and result is to kill. Just use onld signal/no mask

/*	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGALRM);
	sigaddset(&mask, SIGQUIT);
	action.sa_flags=0;
	action.sa_mask=mask;

	action.sa_handler=status;
	sigaction(SIGINT,&action,NULL);
	sigaction(SIGALRM,&action,NULL);


	action.sa_handler=query;
	sigaction(SIGQUIT,&action,NULL);*/
/* Status Routine will handle timer and INTR */
    signal(SIGINT, terminate); //signal(SIGINT, terminate(proInd));
    signal(SIGHUP, terminate);
    signal(SIGQUIT, terminate);

    //now you are all set up and ready to start computing!
    //just be sure that by no chance another process wrote into your space for processs info...
    bitmap->procs[proInd].pid = pid;
    bitmap->procs[proInd].found = 0;
    bitmap->procs[proInd].notTested = 0;
    bitmap->procs[proInd].tested = 0;

    //now Compute computes
    perfect(begin, mesg);
    if (shmdt((char  *) bitmap) == -1) {
        perror("shmdt");
        exit(-1);
    }
    //Compute processes that hit the end should wrap around but STOP AT THEIR STRATING POINT AND EXEC "report -k"
    // according to https://www.geeksforgeeks.org/exec-family-of-functions-in-c/
    char *args[]={"report","-k"};
    execvp(args[0],args);
    //execlp("report","-k");
    //execlp("report", "report", "-k", (char *)NULL);
    //you should not get here...
    return -1;
}










//Computes have to be able to test and set in the bitmap what numbers to check
//the bitmap an array of ints that take up the right number 2^25 bits...
//so to do the bit array and convert numbers to their corresponding bit,
// follow: http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
// to check and set bits:
// https://www.includehelp.com/c/how-to-check-a-particular-bit-is-set-or-not-using-c-program.aspx
int tested( int A[],  int k ) {
    //return 0
    //return ( n & (1 << (k%Size) ) != 0 ) ;
    return ( (A[k/Size] & (1 << (k%Size) )) != 0 ) ;
}
void set( int A[],  int k ) {
    A[k/Size] |= 1 << (k%Size);
}

//for each perfect number founds Compute must report it to Manage
void perfect(int start, struct mesg_buffer mess)
//int start;
{
    int i,j,sum;
    //perfect should only compute up to what can be stored in the array and then wrap around
    for (j = start; j <= BITS; j++ ){
        //make sure that number is not already tested, then check perfect
        //int indB = j/Size;
        if(tested(bitmap->bitMap, j) == 0) {
            //mark it tested
            set(bitmap->bitMap, j);
            bitmap->procs[proInd].tested++; //Test the number for perfect Number
            sum = 1;
            for (i = 2; i < j; i++)
                if (!(j % i)) sum += i;

            //Compute processes are responsible for updating the bit map, as well as their own process statistics.
            if (sum == j) {
                printf("%d is perfect\n", j);
                bitmap->procs[proInd].found++;
                mess.mesg_type = 2;
                mess.perf = j;
                //Compute must send Manage any perfect # found.
                msgsnd(qid, &mess, sizeof(mess.perf),0);
            }

        }
        //then it is not tested
        bitmap->procs[proInd].notTested++;
    }
    printf("going to wrap around\n");
    //if you finish this loop, then that you checked up to the max...

    for (j = 2; j < start; j++ ){
        //make sure that number is not already tested, then check perfect
        //int indB = j/Size;
        if(tested(bitmap->bitMap, j) == 0) {
            //mark it tested
            set(bitmap->bitMap, j);
            bitmap->procs[proInd].tested++; //Testing the number for perfect Number
            sum = 1;
            for (i = 2; i < j; i++)
                if (!(j % i)) sum += i;
            //Compute processes are responsible for updating the bit map, as well as their own process statistics.
            if (sum == j) {
                printf("%d is perfect\n", j);
                bitmap->procs[proInd].found++;
                mess.mesg_type = 2;
                mess.perf = j;
                //Compute must send Manage any perfect # found.
                msgsnd(qid, &mess, sizeof(mess.perf),0);
            }
            n++;
        }
        //then it is not tested
        bitmap->procs[proInd].notTested++;
    }
}

//need to get the process index from manage for the shmem process array
int whatInd(int pid, struct mesg_buffer msg) {
    msg.perf = pid; //storing pid
    //ask and wait for your index
    msgsnd(qid, &msg, sizeof(msg.perf), 0);
    msgrcv(qid, &msg, sizeof(msg.perf), pid, 0);//make sure message is for you
    //printf("New Compute is now computing :)\n");
    proInd = msg.perf;
    return proInd;
}

void
status(signum)
        int signum;

{

    alarm(0); /* shutoff alarm */

    if (signum == SIGINT) printf("Interrupt ");
    if (signum == SIGALRM) printf("Timer ");

    printf("processing %d\n",n);

    alarm(20);	/*restart alarm*/
}

//void query() {siglongjmp(jmpenv,1);}

//Compute processes that hit the end should wrap around but STOP AT THEIR STARTING POINT AND EXEC "report -k"
//All process should terminate cleanly on INTR, QUIT, and HANGUP signals.
//As in delete their process entry from shared memory and terminate.
//This cleans up, ONLY FOR PROCESS THAT GOT IN THE PROCESS ARRAY
void terminate() {
    //delete you proc entry in the bitmap
    //since we want stats for dead processes, leave everything but pid...
    bitmap->procs[proInd].pid = 0;
    //detach the Compute from the bitmap shared memory
    if (shmdt((char  *) bitmap) == -1) {
        perror("shmdt");
        exit(-1);
    }
    exit(0);
}






