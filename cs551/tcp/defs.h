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

//Sent by Manage to Compute
struct inter {
    int start;
    int end;
    int tested;
};

//sent by Compute to request intervals
struct CompReport {
    int threads; //number of threads
    int time; //completion time
    int tested;
};

//sent by Manage to tell Report the perfects and who found them
struct PerfReport {
    int perfect; //perfect found
    char who[20];; //who found it
};

//sent by Manage to report the running Compute to Report
struct ManageReport {
    char who[20];; //who is running
    int tested;
    //int rangeSize;
    int ranges[120]; //enough to store 60 intervals
};