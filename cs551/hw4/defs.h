//according to: https://www.geeksforgeeks.org/write-header-file-c/
// It is not recommended to put function definitions  
// in a header file. Ideally there should be only 
// function declarations. Purpose of this code is 
// to only demonstrate working of header files.

//the shared memory segment should contain the following data:
//bit map large enough for 2^25=33554432 bits. If a bit is off it shows the corresponding int is untested.
//an array of 20 ints to hold the perf numbers found.
//an array "process" structures of length 20

//"proc" struct should contain:
//pid, # of perfect # found, # tested, # not tested
#define Size  sizeof(int)*8 //size of is in terms of char
#define MAX 33554432/Size //number of ints
#define BITS 33554432
#define KEY (key_t)88582	/*key for shared memory segment */


struct proc {
    pid_t pid;
    int found;
    int tested;
    int notTested;
};


struct shMem {
    int bitMap[MAX]; // this is 2^25 bits in terms of ints
    int perf[20];
    struct proc procs[20];
};

// then for IPC, Message Queues according to https://www.geeksforgeeks.org/ipc-using-message-queues/
struct mesg_buffer {
    long mesg_type;
    int perf;
} ;
