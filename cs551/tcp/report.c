//this code is my own work written without an outside tutor or code by other students - Ana Noriega
//based on maryshare from class

//On maryshare.c
/* Program to illustrate sharing memory on System V */
/* Mary will read a vector of numbers in shared memory */

//On report.c
/*
Report only ever speaks to Manage.
 Upon launch, it will ask Manage for a report.
 By default, Report reads inter structs, it uses inter.start to know the type of info it will  be given.
 It uses inter.end to know the size of the next info it will read.
 It prints this info.
 if it got a -k, it tells Manage to end everything by sending it int -2
 Otherwise, Report peacefully dies at the end.
*/

//read in the form of inter to avoid mix-ups

//interval.start=1;
//                        interval.end = sizeof(perfects);
//                        //write this out
//                        write(pollarray[i].fd,&interval, sizeof(struct inter));
//                        //now that report knows what to expect, send the perfects
//                        write(pollarray[i].fd,&perfects, sizeof(perfects));
//                        //now send it an iter struct to let it know you are sending the compute info, and the size
//                        interval.start=2;

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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>


struct PerfReport perfects[52];
struct ManageReport comps[12];

void compPrint(int arr[120]);





int main(int argc, char* argv[]) {

    /* from class tcp examples*/
    struct sockaddr_in sin; /* socket address for destination */
    long address;
    int i,j,s;
    struct inter interval;


    if (argc < 3 ) {
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
    //now that you are connected, send Manage a request and then read for the inter struct that tell you how big to read
    j=htonl(-4);
    write(s, &j, sizeof(int));
    //now that Manage knows we want the reports, wait for it to send inters with the info
    while (1){
        int done = 0;
        read(s, &interval, sizeof(struct inter));
        int type = interval.start;
        int size = interval.end;
        if (type==1){//you will be sent the perfects
            printf("The Perfect Numbers are:");
            read(s, &perfects, size);
            for (int k = 0; k < 52 ; ++k) {
                if(perfects[i].perfect != 0){
                    printf("%d    %s\n", perfects[i].perfect, perfects[i].who );
                }

            }
            done++;
        }
        //The compute info
        if (type==2){
            printf("Computes:\nHost          Tested      Ranges\n");
            read(s, &comps, size);
            for (int k = 0; k < 12 ; ++k) {
                if(comps->ranges[0] != -1){//only for active processes
                    printf("%s    %d       ", comps->who, comps->tested );
                    //the print the intervals
                    compPrint(comps->ranges);
                }
            }
            done++;
        }
        if (done>1) break;
    }


    if((argv[3]!= NULL) && strcmp(argv[3], "-k") == 0) {
        j=htonl(-2);
        write(s, &j, sizeof(int));
    }
    // otherwise report has nothing left to do
    exit(0);

}


void compPrint(int arr[120]){
    //given and array of ranges, print the pairs until you get to the empty part
    for (int i = 0; i <120 ; i=i+2) {
        printf(" %d-%d   ", arr[i],arr[i+1]);
        if(arr[i+2]==0){
            printf("\n");
            break;
        }
    }

}
