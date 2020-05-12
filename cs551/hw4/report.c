//this code is my own work written without an outside tutor or code by other students - Ana Noriega
//based on maryshare from class

//On maryshare.c
/* Program to illustrate sharing memory on System V */
/* Mary will read a vector of numbers in shared memory */

//On report.c
/*
Report's job is to read the shared memory segment and report on:
 the perfect numbers found, total numbers tested, and for each process currentyly computing: (# tested, #skipped, # found)
It should also give a total of these three numbers that includes processos no longer running.
If invoked with the "-k" switch, it also is used to inform Manage to shut down computation.
Terminate on INTR, QUIT, or HANGUP.
If you get the -k flag, print the report and send an INTR to "manage"
*/

#include <sys/msg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "defs.h" //the new header
#define KEY (key_t)88582	/*key for shared memory segment */


struct shMem *bitmap;
int sid;//shared mem
int qid; //message queue
//Report must send Manage messages.


void terminate();
int perfects( struct mesg_buffer msg);
long TotalTest( );
long countSetBits( int n);






int main(int argc, char* argv[]) {

    //int sid;	/* segment id of shared memory segment */
    //int *array;	/* pointer to shared array, no storage yet*/
    //int j;		/*loop counter */
    //int sum; 	/*running sum*/



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
    int Perfects = 0; //shmem still not read...
/*

    //Terminate on INTR, QUIT, or HANGUP.
    signal(SIGINT, terminate); //signal(SIGINT, terminate(proInd));
    signal(SIGHUP, terminate);
    signal(SIGQUIT, terminate);
*/


    //Report's job is to read the shared memory segment and report on:
    //the perfect numbers found, total numbers tested, and for each process currentyly computing: (# tested, #skipped, # found)
    //It should also give a total of these three numbers that includes processos no longer running.


    /* Now add it up */
    //Again, perfect numbers are so rare that we assume the array never fills...
    printf("\nOur perfect numbers so far: \n");
    for (int i = 0; i < 20; i++){
        int num = bitmap->perf[i];
        if (num != 0){
            printf("%d\n ", num);
            Perfects++;
        }
    }




    //and for each process currently computing: (# tested, #skipped, # found)
    int sum = 0;
    int sumF =0;
    int sumS = 0;

    printf("\n\n\nFor each current Compute process:\n");
    for (int j = 0; j < 20; j++) {
        //this way you get shut down processes too
        sum = sum + bitmap->procs[j].tested;
        sumF = sumF + bitmap->procs[j].found;
        sumS = sumS + bitmap->procs[j].notTested;
        if(bitmap->procs[j].pid != 0){
            printf("pid: %d   tested: %d   skipped: %d   found:%d   \n", bitmap->procs[j].pid, bitmap->procs[j].tested,
                    bitmap->procs[j].notTested, bitmap->procs[j].found );
        }

    }
    //total tested
    printf("\nThe total tested for running: %d", sum);
    long allT = TotalTest();
    printf("\nThe total tested from bitmap: %ld", allT);
    printf("\nTotal found by array: %d", Perfects);
    printf("\nTotal found by all proc: %d", sumF);
    printf("\nTotal skipped: %d\n", sumS);
    //if you were called with k, tell Manage and die

    if((argv[1]!= NULL) && strcmp(argv[1], "-k") == 0) {
        printf("\n\nThat was a -k\n\n");
        struct mesg_buffer msg;
        msg.mesg_type = 3;
        msg.perf = 0;
        msgsnd(qid, &msg, sizeof(msg.perf), 0);
        /* Unmap and deallocate the shared segment */
        terminate();
    }
    // otherwise report has nothing left to do
    exit(0);

}


//total tested is the total bits set in the bitmap
// https://www.geeksforgeeks.org/count-set-bits-in-an-integer/
long countSetBits( int n)
{
    long count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

long TotalTest( ){
    long totes = 1;
    for (int i = 0; i < MAX ; ++i) {
        int B = bitmap->bitMap[i];
        totes = totes + countSetBits(B);
    }
    return totes;

}

void terminate() {
    //detach the Compute from the bitmap shared memory
    /* Unmap and deallocate the shared segment */
    if (shmdt((char *) bitmap) == -1) {
        perror("shmdt");
        kill(getpid(),SIGKILL);
    }
}
