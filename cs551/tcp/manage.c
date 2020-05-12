//this code is my own work written without an outside tutor or code by other students - Ana Noriega
//based on johnshare from class

//I AM NOT DONE WITH THESE BUT MY INTERNET IS HAVING PROBLEMS, SO JUST IN CASE IT GOES DOWN I AM UPLOAD

//On johnshare.c
/* Program to illustrate sharing memory on System V */
/* John will create a vector of numbers in shared memory */

//On manage.c
/*
Manage's job is to coordinate the computations done by the Computes.
It is the first process started and takes just one argument, the tcp port number to be used for communication.
Manage starts up, does any initialization and waits for connections.

Manage hands out untested search ranges, and checks times to target 15 seconds.
Manage records the perfects found and who found them.
Manage records host names of running processes, the amount of numbers tested,
the current range it is working on.
The number tested means asking the present number of the running threads.

On a INTR, QUIT, or HANGUP send an INTR to all running computes, sleeps 5 seconds,  and die.

 Report will ask Manage to tell it all the perfect numbers found and the host that found them,
 For the running Computes what host it is on, how many numbers it has tested, current interval it is testing
*/


//NOTE ON COMMUNICATING: MANAGE RECEIVES REQUESTS FOR INTERVALS, KILL SIGNALS, AND PERFECT NUMBERS
//AS PERFECT NUMBERS CANNOT BE < 0, THE WAY THAT SPECIAL REQUESTS ARE IDENTIFIED IS THAT THEY ARE LESS THAN ZERO.
//SO, WHEN MANAGE GETS A MESSAGE IT TESTS >0, THEN ==-1, ==-2
//FOR > 0, IT RECORDS THE PERFECT
//FOR -2, IT JUST SENDS A KILL SIGNAL
//FOR -1, IT THEN READS THE REPORT STRUCT and sends out an interval as an inter structure
//FOR -3, MANAGES NOTES THAT THREAD TERMINATED
//FOR -4, THIS IS FROM REPORT. MANAGE SENDS IT ALL THE PERFECT REPORTS AND THE CURRENT INTERVALS IT IS TESTING

//Arguments are port

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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/wait.h>
#include <poll.h>

//so terminate can use it
struct pollfd pollarray[13];
struct inter interval;




void terminate();


int processes;
int main(int argc, char* argv[]) {

    struct sockaddr_in sin; /* structure for socket address */
    int s;
    int fd;
    int len, i,num,action;
    struct hostent *hostentp;
    //this is where the found perfect will be stored
    //there are only 51 know perfects as of 2018
    struct PerfReport perfects[52];
    //no more than 10 Computes
    struct ManageReport comps[12];
    int pIndx = 0;
    struct CompReport Requests;
    int start = 1;
    int step = 1000;
    int rangeInx[13];

    /* set up IP addr and port number for bind */
    sin.sin_addr.s_addr= INADDR_ANY;
    sin.sin_port = htons(atoi(argv[1]));
    sin.sin_family= AF_INET;

    /* Status Routine will handle timer and INTR */
    signal(SIGINT, terminate); //signal(SIGINT, terminate(proInd));
    signal(SIGHUP, terminate);
    signal(SIGQUIT, terminate);

    /* Get an internet socket for stream connections */
    if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("Socket");
        exit(1);
    }

    /* Do the actual bind */
    if (bind (s, (struct sockaddr *) &sin, sizeof (sin)) <0) {
        perror("bind");
        exit(2);
    }

    /* Allow a connection queue for up to 10 Computes and 1 Report */
    listen(s,12);

    /* Initialize the pollarray */
    pollarray[0].fd=s;     /* Accept Socket*/
    pollarray[0].events=POLLIN;
    /* 10 possible Computes and 1 Report */
    for (i=1;i<=12;i++) {pollarray[i].fd=-1;pollarray[i].events=POLLIN;}

    while(1) {
        poll(pollarray,12,-1);   /* no timeout, blocks until some activity*/

        /* Check first for new connection */
        if (pollarray[0].revents & POLLIN) {

            len=sizeof(sin);
            if ((fd= accept (s, (struct sockaddr *) &sin, &len)) <0) {
                perror ("accept");
                exit(3);
            }
            /* Find first available slot for new john */
            for (i=1;i<=12;i++) if (pollarray[i].fd == -1) break;
            pollarray[i].fd=fd;
            hostentp=gethostbyaddr((char *)&sin.sin_addr.s_addr,
                                   sizeof(sin.sin_addr.s_addr),AF_INET);
            strcpy((char *)&comps[i].who, hostentp->h_name);
        }

            /* If there are no new connections, process waiting Processes */
        else for (i=1;i<=12;i++) {
                if ((pollarray[i].fd !=-1) && pollarray[i].revents) {
                    //All of the action work by sending an int first, then depending on that int's value
                    //Manage will act diffrently
                    action=read(pollarray[i].fd,&num, sizeof(int));
                    // It's a perfect
                    if (action>0) {//it's a perfect
                        perfects[pIndx].perfect=action;
                        snprintf(perfects[pIndx].who, 7, "Host%d", i); //store 2 digits and terminating null
                        //incrament where you store the next perfect
                        pIndx++;
                    }
                    if (action==-1) {//a compute wants intervals. Read the Report struct, and send out an inter structure
                        //read the report
                        read(pollarray[i].fd,&Requests, sizeof(struct CompReport));
                        //check the time
                        if (Requests.time > 15) {
                            step = step/2;
                        }
                        //check the time
                        if (Requests.time < 15) {
                            step = step*2;
                        }
                        if (comps[i].ranges[0]==0) { //If you have not yet recorder this process as running
                            comps[i].tested += Requests.tested;
                            //comps[i].rangeSize += sizeof(int);
                        };
                        //then, based of the number needed, send out intervals
                        for (int j = 0; j < Requests.threads; ++j) {
                            // then send out that number of intervals
                            interval.start = start+1;
                            comps[i].ranges[rangeInx[i]]=start+1;
                            rangeInx[i]++;
                            int end = start + step;
                            interval.end = end;
                            comps[i].ranges[rangeInx[i]]=end;
                            rangeInx[i]++;
                            start = end;
                            write(pollarray[i].fd,&interval, sizeof(struct inter));
                        }
                    }
                    if (action==-2) {//FOR -2, IT JUST SENDS A KILL SIGNAL
                        //then, based of the number needed, send out intervals
                       terminate();
                        }
                    }
                    if (action==-3) {//a thread terminated, mark its Compute info blank
                        comps[i].tested = NULL;
                        rangeInx[i] = 0;
                        comps->ranges[0]=-1;
                        snprintf(comps[i].who, 20, NULL);
                    }
                    if (action==-4) {//this is report requesting info
                        //first send it an iter struct to let it know you are sending the perfects, and the size
                        interval.start=1;
                        interval.end = sizeof(perfects);
                        //write this out
                        write(pollarray[i].fd,&interval, sizeof(struct inter));
                        //now that report knows what to expect, send the perfects
                        write(pollarray[i].fd,&perfects, sizeof(perfects));
                        //now send it an iter struct to let it know you are sending the compute info, and the size
                        interval.start=2;
                        interval.end = sizeof(comps);
                        //write this out
                        write(pollarray[i].fd,&interval, sizeof(struct inter));
                        //now that report knows what to expect, send the cmps
                        write(pollarray[i].fd,&comps, sizeof(comps));
                    }

                }

                }
            }




//On a INTR, QUIT, or HANGUP send an INTR to all running computes, sleeps 5 seconds, then deallocates the shared memory and die.
void terminate(){
    //send an INTR to all running computes
    for(int i = 0; i < 11; i++) {
        interval.start = -2;
        interval.end = -2;
        write(pollarray[i].fd,&interval, sizeof(struct inter));
    }

    // then sleep 5 seconds
    sleep(5);
    //die
    kill(getpid(),SIGKILL);
}

