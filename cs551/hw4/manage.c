//this code is my own work written without an outside tutor or code by other students - Ana Noriega
//based on johnshare from class

//On johnshare.c
/* Program to illustrate sharing memory on System V */
/* John will create a vector of numbers in shared memory */

//On manage.c
/*
Manage's job is to maintain the shared memory segment.
The shared segment is where the compute processes post their results.
It also keeps track of active Compute proc so it can signal them to terminate.
Manage initalizes the process entry for each Compute.
Use whatever IPC for Compute registering with manage.
Manage updates the array of perf num.
On a INTR, QUIT, or HANGUP send an INTR to all running computes, sleeps 5 seconds, then deallocates the shared memory and die.
*/

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <signal.h>
#include <string.h>
#include "defs.h" //the new header
#define KEY (key_t)88582	/*key for shared memory segment */



//int n; /* global variable indicating current test point */

//they keep process statistics and write to shared memory, multiple Computes can run
struct shMem *bitmap;
int sid;//shared mem
int qid; //message queue

void newComp(struct mesg_buffer msg);
void terminate();
int perfects( struct mesg_buffer msg, int Perfects);


int main() {
    //int sid;	/* segment id of shared memory segment */
    //int *array;	/* pointer to shared array, no storage yet*/
    //int j;		/*loop counter */
    /*
    char *shmat();
    */


    /* create queue if necessary */
    if ((qid = msgget(KEY, IPC_CREAT | 0660)) == -1) {
        perror("msgget");
        exit(1);
    }
    /* create shared segment if necessary */
    if ((sid = shmget(KEY, sizeof(struct shMem), IPC_CREAT | 0660)) == -1) {
        perror("shmget");
        exit(1);
    }
    /* map it into our address space*/
    if ((bitmap = ((struct shMem *) shmat(sid, 0, 0))) == (struct shMem *) -1) {
        perror("shmat");
        exit(2);
    }

    /* Status Routine will handle timer and INTR */
    signal(SIGINT, terminate); //signal(SIGINT, terminate(proInd));
    signal(SIGHUP, terminate);
    signal(SIGQUIT, terminate);

    int Perfects; //the number of
    int Last; //index of the last perfect found in Array
    //Compute must send Manage any perfect # found.

    int proInd;
    Perfects = 0; //no perfects have been found yet...
    Last = 0;
    //you have just started, so set the shared mem to be blank
    //https://www.geeksforgeeks.org/memset-c-example/
    memset(bitmap, 0, 33554432);
    struct mesg_buffer mesg;

    //then just keep looking for messages!
    while (1) {
        //treat checking a found perfect as more important than  a "report -k"!
        //actually report k as least important so it dose not leave zombies...
        msgrcv(qid, &mesg, sizeof(mesg.perf), -3, 0);
        switch (mesg.mesg_type) {
            case 1:
                newComp(mesg);
                break;
            case 2:
                Perfects = perfects(mesg, Perfects);
                break;
            case 3:
                terminate( );
                break;
            default:
                printf("weird...\n");
                break;
        }
    }

}

//It also keeps track of active Compute proc so it can signal them to terminate.

//Manage initalizes the process entry for each Compute.
//Use whatever IPC for whenever a new Compute is registering with Manage.
//https://www.includehelp.com/c/process-identification-pid_t-data-type.aspx
void newComp( struct mesg_buffer msg) {
    //looping through the Process Array to find a spot
    int i;
    int pid = msg.perf;
    for(i =0; i < 20; i++) {
        //see which Proc Structs say they are empty...
        //0 means not initialized
        if((int)(bitmap->procs[i].pid) == 0) {
            //stop looking and tell Compute
            //msg.mesg_type = msg.perf;//so the pid tell which comp to read it
            msg.perf = i;
            printf("\nFound slot for process at %d\n",i);
            break;
        }
    }
    //if you do the whole loop, i is now 20 > 19
    //if you have not done the whole loop i < 19
    //so the pid tell which comp to read it
    msg.perf = i;
    msg.mesg_type = pid;
    printf("\nSending slot for process at %d  %d\n",(int)msg.perf, (int)msg.mesg_type);
    //either way let Compute know
    msgsnd(qid, &msg, sizeof(msg.perf), 0);
}

//Manage updates the array of perf num.
//when Compute finds a perfect, it tells Manage
int perfects( struct mesg_buffer msg, int Perfects) {
    //check if a different Compute already found this...
    //assume the only case where two Computes would have tested the same bit before one can set it
    // is if they are running at the same index taking about the same amount of time...
    // as most perfect numbers are not that fast to find, the array of the twenty
    // most recent should still have this offending number...
    // Therefore, lets see it is good enough to check the array for repeats
    int i;
    int new = 1; //assume it's new
    int perf = msg.perf;
    for (i=0; i < 20; i++) {
        //not new. stop checking
        if (bitmap->perf[i] == perf) {
            new = 0;
            break;
        }
    }
    //it's new! You went through all 20 Perfects and it wasn't there, so it is time to insert
    //There are only 51 perfect numbers, with the latest being 49,724,095 digits...
    // So, we won't even worry about the case that we overwrite a perfect
    if(new == 1) {
        //insert it right after the last perfect number found.
        //This is the total number of perfects before incrementing
        //This way oldest numbers are pushed out last- meaning their bitmap is likeliest to be updated by now
        int ind = Perfects%20;
        bitmap->perf[ind] = perf;
        Perfects++;
    }
    return Perfects;
}


//On a INTR, QUIT, or HANGUP send an INTR to all running computes, sleeps 5 seconds, then deallocates the shared memory and die.
void terminate( ){
    //send an INTR to all running computes
    for(int i = 0; i < 20; i++) {
        int pid = bitmap->procs[i].pid;
        // if there is a running / filled out proc structure
        if(pid != 0) {
            kill(pid, SIGINT);
            printf("killed %d \n", pid);
        }
    }

    // then sleep 5 seconds
    sleep(5);
    //deallocate shared memory and die...
    /* Unmap and deallocate the shared segment */

    if (shmdt((char  *) bitmap) == -1) {//detach
        perror("shmdt");
        exit(3);
    }
    if (shmctl(sid,IPC_RMID,NULL) == -1) {//delete
        perror("shmctl");
        exit(3);
    }
    printf("\nShared Mem delete!");
    if (msgctl(qid, IPC_RMID, NULL) == -1) { //remember to get rid of  message  queue!
        perror("msgctl");
        exit(3);
    }
    printf("\nM Que delete!\n");
    //die
    kill(getpid(),SIGKILL);
}

