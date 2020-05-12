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

//THIS IS JUST MODIFIED HW4
//What is diffrent:
/*
compute takes 3 args: host name, port for manage and number of threads to launch.
each thread is given a search range- as no need to check in bit map.
compute sends manage the perfects it finds when it finds  them.
*/


//NOTE ON COMMUNICATING: MANAGE RECEIVES REQUESTS FOR INTERVALS, KILL SIGNALS, AND PERFECT NUMBERS
//AS PERFECT NUMBERS CANNOT BE < 0, THE WAY THAT SPECIAL REQUESTS ARE IDENTIFIED IS THAT THEY ARE LESS THAN ZERO.
//SO, WHEN MANAGE GETS A MESSAGE IT TESTS >0, THEN ==-1, ==-2
//FOR > 0, IT RECORDS THE PERFECT
//FOR -2, IT JUST SEND A KILL SIGNAL
//FOR -1, IT THEN READS THE REPORT STRUCT
//FOR -3, MANAGES NOTES THAT THREAD TERMINATED

//Arguments are host, port, threads

//based on hw4 and john3tcp
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
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

int s;
//struct inter interval[10];
void *perfect();
//void perfect(struct inter st);
void terminate();
//a global to let threads know to tell Manage how many number they read of this interval
int tell;
int tested;



int main(int argc, char* argv[]) {
    int  threads; /* starting point for next search, and the  info for tcp*/
    /* from class tcp examples*/
    struct sockaddr_in sin; /* socket address for destination */
    int len;
    long address;
    int i,j;
    pthread_t tid;
    struct inter interval;
    struct CompReport Req;

    if (argc < 4 ) {
        printf("too few args.\n");
        exit(1);
    }
    /* Fill in Mary's Address */
    //this happens in place of the usual "if (//assign val fxn =-1) error...
    address = *(long *) gethostbyname(argv[1])->h_addr;
    sin.sin_addr.s_addr= address;
    sin.sin_family= AF_INET;
    sin.sin_port = htons(atoi(argv[2]));

    while(1) { /*loop waiting for Mary if Necessary */
        /* create the socket */
        if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
            perror("Socket");
            exit(1);
        }

        /* try to connect to Mary */
        if (connect (s, (struct sockaddr *) &sin, sizeof (sin)) <0) {
            printf("Where is that Manage!\n");
            close(s);
            sleep(10);
            continue;
        }
        break; /* connection successful */
    }


    //all computes should terminate on certain signals
    signal(SIGINT, terminate); //signal(SIGINT, terminate(proInd));
    signal(SIGHUP, terminate);
    signal(SIGQUIT, terminate);

    //Compute connects to Manage and sends it a start up request, including the number of threads it has
    Req.threads = atoi(argv[3]);
    Req.time = 0;
    j=htonl(-1);
    write(s, &j, sizeof(int));
    threads = atoi(argv[3]);
    write(s, &Req, sizeof(struct CompReport));
    //Mange gives a range for each thread in the form of two consecutive integers written to this Compute's fd
    //read these two, while launching the threads
    for (i = 0; i < threads; i++) {
         read(s, &interval, sizeof(struct inter));
         //read(s, &interval.end, sizeof(int));
         //read(s, &interval.fd, sizeof(int));
         //threads only take one arg...
        pthread_create(&tid, NULL, perfect, &interval);
    }

    //the handeling of getting a new range is in perfect
    //wait on your threads
    //actually from how perfect is writen it should never fininsh, but intead kill the process
    //the case that there is no more interval will have to fall on Manage to send out a kill
    for (i = 0; i < threads; i++) {
        //pthread_join();
    }
    //as the threads are the ones reading new numbers, it falls on them to detect Manage's kill signal
    //you should not get here
    //exit(1);


}







//for each perfect number founds Compute must report it to Manage
//after being attached, for TCP send and receive is done by read/ write
//for this you need an int s from the socket to write, and an int fd to read
//this function perfect is called within each thread
//void perfect(int start, int end, int fd)
void *perfect(struct inter *st)
{
    int i,j,sum;
    struct CompReport Report;
    //be sure there is no kill signal
    int start = st->start;
    int end = st->end;
    int fd = st->tested;
    if (start==-2|end==-2){
        kill(getpid(),SIGKILL);
    }
    // to time the interval
    clock_t t;
    t = clock();


    while (1) {

        for (j = start; j <= end; j++) {
            //find the perfects
            sum = 1;
            for (i = 2; i < j; i++)
                if (!(j % i)) sum += i;

            //If Compute found a perfect, send it to Manage
            if (sum == j) {
                printf("%d is perfect\n", j);
                // immediately send the number to manage
                //manage also has to know what host found it
                //from mary tcp, it seems that the loop to accept sockets in Manage does so to diffrent fd each time
                //so there is no need to send who found the number here...
                //Compute must send Manage any perfect # found.
                write(fd, &j, sizeof(j));
            }

        }
        //after you finish this range, ask Manage for a new one
        //Write out a special message
        //As -1 would never be perfect, this can be used as a flag for Manage that a new range.
        //Each Compute would be it's own fd or fd[i], so we know computes won't collide.
        //But will threads collide?
        //A thread would only write to the socket if it has a perf or has finished it's range.
        //A Compute would read from it's socket fd if it is either starting up or if a thread needs a new range
        //So, as these are similar, the way that Manage should pass the ranges is as consecutive ints
        //It is never the case that Compute tells Manage to die because "the bitmap is done"
        //So, if a thread blocks because Manage has not sent it a new range we don't care.
        //On finishing your range, read a new pair of end points
        t = clock() - t;
        Report.time = (int)(((double)t)/CLOCKS_PER_SEC);
        Report.threads = 1; //in case threads>0, Manage loops to send that many intervals and update # threads
        //else Manage just sends one interval
        j=htonl(-1);
        write(fd, &j, sizeof(int));
        write(fd, &Report, sizeof(struct CompReport));
        // The absolute only thing that Manage will ever write out is ranges to check
        // Or the kill signal (int the form of intervals)
        // As such, we stop and think what are the dangers that could happen?
        // The only error I could think of is that threads mix-up messages meant for each other.
        // There would only be a real mix-up if the numbers fetched for the range did not make sense...
        // To avoid that, all ranges come packaged in a struct. That struct has the matched start and end points
        read(fd, &st, sizeof(struct inter));
        if (st->start==-2){
            kill(getpid(),SIGKILL);
        }
        if (st->end==-2){
            kill(getpid(),SIGKILL);
        }
    }
}




//All process should terminate cleanly on INTR, QUIT, and HANGUP signals.
//As in stop computing and tell Manage that they are terminating
void terminate() {
   //tell Manage
    int j=htonl(-3);
    write(s, &j, sizeof(int));
    sleep(5);
    kill(getpid(),SIGKILL);
}






