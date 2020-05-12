//this code is my own work written without an outside tutor or code by other students - Ana Noriega


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
//#include <ar.h>
#include <string.h>
#include <dirent.h>
#include <utime.h>


#define  ARMAG   "!<arch>\n"    /* magic string */
#define  SARMAG   8             /* length of magic string */


#define  ARFMAG   "`\n"         /* header trailer string */

//the meta structure we were told to use
struct meta {  /* definition of new type "meta"*/
    char  name[16];
    int mode;
    time_t mtime; //a long
    int size;
}meta;

//
struct  ar_hdr                  /* file member header */
{
    char    ar_name[16];        /* '/' terminated file member name */
    char    ar_date[12];        /* file member date */
    char    ar_uid[6];           /* file member user identification */
    char    ar_gid[6];          /* file member group identification */
    char    ar_mode[8];          /* file member mode (octal) */
    char    ar_size[10];        /* file member size */
    char    ar_fmag[2];         /* header trailer string */
}ar_hdr;


int HDSIZE = (sizeof(struct ar_hdr));
int MSIZE = (sizeof(struct meta));
// the helper functions we were told to use



// does stat on the file name in order to fill in the ar_hdr given to it
// this requires converting from binary to ascii
// fxn is used in adding members to the archive
int fill_ar_hdr(char *filename, struct ar_hdr *hdr){
    // stat the filename
    struct stat fileStat;
    stat(filename, &fileStat);

    int use = 0;
    if (fileStat.st_mode & S_IRUSR){use = use + 4;}
    if (fileStat.st_mode & S_IWUSR){use += 2;}
    if (fileStat.st_mode & S_IXUSR){use += 1;}

    //printf("m: %d\n", use);

    int grp = 0;
    if (fileStat.st_mode & S_IRGRP){grp = grp + 4;}
    if (fileStat.st_mode & S_IWGRP){grp += 2;}
    if (fileStat.st_mode & S_IXGRP){grp += 1;}

    //printf("m: %d\n", grp);

    int oth = 0;
    if (fileStat.st_mode & S_IROTH){oth = oth + 4;}
    if (fileStat.st_mode & S_IWOTH){oth += 2;}
    if (fileStat.st_mode & S_IXOTH){oth += 1;}

     //printf("m: %d\n", oth);
    // then to make string
    char mode[8];
    sprintf(mode, "0%d%d%d", use, grp, oth);
    //printf("%d%d%d foo", use, grp, oth);
    //printf("%s \n", mode);
    //hdr->ar_mode = mode;
    strcpy(hdr->ar_mode, mode);
    //write(hdr->ar_mode,mode, 8);
    printf("%s \n",hdr->ar_mode);


    //printf("The file %s a symbolic link\n", (S_ISLNK(fileStat.st_mode)) ? "is" : "is not");
    // arhdr needs
    //name
    char name[16];
    sprintf(name,"%s/", filename);
    strcpy(hdr->ar_name, name);
    //printf("%s", hdr->ar_name);
    //date
    //hdr->ar_date = fileStat.st_mtim;
    //strcpy(hdr->ar_date , fileStat.st_mtim);
    char time[12];
    sprintf(time,"%lu", fileStat.st_mtim);
    strcpy(hdr->ar_date, time);
    //printf("%s", hdr->ar_date);
    //uid
    char id[6];
    sprintf(id,"%d", fileStat.st_uid);
    //printf("uid: %s \n", id);
    strcpy(hdr->ar_uid, id);
    //printf("%s", hdr->ar_uid);
    //gid
    char gid[6];
    sprintf(gid, "%d", fileStat.st_gid);
    strcpy(hdr->ar_gid , gid);
    //printf("%s \n", gid);
    //printf("%s", hdr->ar_gid);
    //mode, as in 644
    //strcpy(hdr->ar_mode , mode);
    //printf("mode: %s \n", mode);
    //size
    char size[10];
    sprintf(size,"%lu", fileStat.st_size);
    strcpy(hdr->ar_size, size);
    //printf("%s",hdr->ar_size);
    //fmag is `\n
    strcpy(hdr->ar_fmag, ARFMAG);
    //printf("%s", hdr->ar_fmag);

}



// this goes in the oposite direcrion,
int fill_meta( struct ar_hdr *hdr, struct meta *met){
    // meta has
    char  name[16];
    strcpy(met->name, hdr->ar_name);
    //printf("\nmeta %s \n", met->name);
    char mo[8];
    strcpy(mo, hdr->ar_mode);
    //printf("%s", hdr->ar_mode);
    met->mode = atoi(mo);
    //printf(" %s \n", hdr->ar_mode);
    //int mode = atoi(hdr->ar_mode);
    //printf("int m: %d \n", met->mode);
    time_t mtime =atol(hdr->ar_date);//a long
    met->mtime = mtime;
    printf("%lu ", met->mtime);
    int size = atoi(hdr->ar_size);
    met->size = size;
    printf("%d \n", size);


}

//then the helper functions for each of the commands

//append a single file to the archive


//given a pointer/fd to the archive, a pointer to a file and a buffer pointer
// this appends that file
void append(int arpt, char *filename, char *buffer, int bufS ){
    //create the header
    struct ar_hdr *bufH = (struct ar_hdr *) malloc(HDSIZE);
    fill_ar_hdr(filename, bufH);
    //printf("h: %s\n", bufH->ar_name);
    //then as long as the buffer can fit it, write the file into the buffer
    int fd;
    fd = open(filename, O_RDONLY);
    if (fd == -1){
        printf("can't open file %s , skip\n", bufH->ar_name);
        return;
    }
    //write the header to the buffer
    //char *hd = {bufH->ar_name, bufH->ar_date, bufH->ar_uid, bufH->ar_gid, bufH->ar_mode, bufH->ar_size, bufH->ar_fmag};
    //printf("%s", bufH+16+12);
    //this extra write only happens once per append, not big deal
    write(arpt, bufH, HDSIZE);
    //memcpy(buffer,bufH, HDSIZE);
    //printf("%.30s", buffer+16+12+6+6+8);
    // buffer now has the header in it, and we do not want to overwrite header, so our very first read and write will be
    // done with read's buffer start point adjusted to account for the header
    int iosize;
    //iosize = read(fd, buffer, bufS);
    int size = atoi(bufH->ar_size) ;
    printf("head s: %d", size);
    //printf("%.30s", buffer+16+12+6+6);
    //printf("%.30s", buffer+HDSIZE);
    // now that the header and as much file as posiable are on the buffer, write the buffer to ar
    //NOTE arfd was opened with the append flag, so it will automatically append
    //write(arpt, buffer,iosize+HDSIZE);
    // this first write was done special because of not wanting to overwrite the header
    // all of the other writes can be done according to the copy.c example
    while ((iosize=read(fd,buffer, bufS)) >0 ) {
        //if you are on your final write and the file size was odd, add a padding bit
        if (iosize<bufS){
            // if the size is odd, ar seems to add a new line
            if (size%2 == 1){
                //write a padding bit to the buffer
                memcpy(buffer+iosize,"\n", 1);
                iosize += 1;

            }
             //offsets are on even numbers
        }

        write(arpt, buffer, iosize);
        //printf("%.50s\0", arpt);
    }
    // when you are done, close the file discriptor NEW
    close(fd);
}


// a helper for remove, it checks if a string is in a list of strings
// if it is, then it sets that string in the char** to null to avoid further matches and then returns 1
// else, it returns zero
int inList(char** list, int len, char *name){
    int i = 0;
    int f = -1;
    while (i < len){
        //printf("%s", strcat(list[i],"/"));
        char dum[16];
        strcpy(dum,  list[i]);
        if(strcmp(strcat(dum,"/"), name) ==0){
            f = 1;
            // this should never match a file name
            strcpy(list[i],"!+++/\n");
            return 1;
        }
        i++;
    }
    return f;
}

int min(int a, int b){
    if (a<b){
        return a;
    } else return b;
}

int del(int ahd, int dsize, int BufS, char *an, int p2){
    //if found, you have the ar pointer and the size of the file to delete
    //to not lose space you need to create a new file for ar
    //for this new ar, change the name and permisssions at the end
    int nar = open("tempart",O_WRONLY | O_CREAT |O_APPEND,0666);
    struct stat arStat;
    stat(an, &arStat);
    //then you will want to copy the original archive from the very begining upto just before the offending header was read
    // here, the value dsize tells how many bits to write before running into the file to delete, dsize 2 says the offset
    // to use for writing the remainder of the file
    int  iosize;
    int readB = 0;
    //printf("%d", dsize);
    lseek(ahd, 0, SEEK_SET);
    char buffer[BufS];
    int wriS = min(dsize, BufS);
    // if your buffer is larger than the ar to copy, then just do an ar sized write
    while (readB < dsize ) {
        iosize=read(ahd,buffer, wriS);
        readB += iosize;
        //if you are on your final write and the file size was odd, add a padding bit
        write(nar, buffer, iosize);
        wriS = min(wriS,dsize-readB);
        //printf("%d %d %d\n\0", readB, dsize, iosize);
    }
    //after the above loop, everything before the file to delete has been added to temp.
    //now to add everything after
    //set the pointer offset to be after the bad file and it's header
    // that is the p2  as passed
    //printf("%d %d ", readB, readB+HDSIZE+p2);
    lseek(ahd, readB+HDSIZE+p2, SEEK_SET);
    //char f[3] = "foo";
    //write(nar, f, 3);
    // now you can just copy as long as there is something to copy
    int read2 = 0;
    while ((iosize=read(ahd,buffer,BufS)) >0 ) {
        read2 += iosize;
        write(nar, buffer, iosize);
    }
    //after the tail of the file has been written on, move the pointer back to the last file that had it's header checked
    //lseek(nar, readB, SEEK_SET);
    //then, after all the appends are done as need be
    //unlink original
    ahd = nar;
    unlink(an);
    rename("tempart", an);
    ahd = open(an, O_APPEND|O_RDWR, arStat.st_mode);
    lseek(ahd, 0, SEEK_SET);
    //write(ahd, "f62", 2);
    return ahd;
}


void ext(int ahd, int BufS, char *fname, int perm, int fsize, int flag, long ftime, int gid, int uid){
    //if found, you have the ar pointer and the header of the file to copy
    // the file to extract
    printf("%s", fname);
    int fd = open(fname,O_WRONLY | O_CREAT |O_APPEND,0666);
    //then you will want to copy the original archive from the part where it's header ends  for size bits
    int  iosize;
    int readB = 0;
    char buffer[BufS];
    int wriS = min(fsize, BufS);
    // if your buffer is larger than the file to copy, then just do an file sized write
    while (readB < fsize ) {
        iosize=read(ahd,buffer, wriS);
        readB += iosize;
        //if you are on your final write and the file size was odd, add a padding bit
        write(fd, buffer, iosize);
        wriS = min(wriS,fsize-readB);
        //printf("%d %d %d\n\0", readB, dsize, iosize);
    }
    //set the file according to the header
    chmod(fname, perm);
    chown(fname, gid, uid );
    if (flag){
        const struct utimbuf *timeN = {(time_t) ftime, (time_t) ftime };
        utime(fname, timeN);
    }
    close(fd);
}



//recives a file descriptor to the archive and prints a table of contents
//in ar the table of contents is just fileName\n file\n ...
void t(int ar){
    //you are given the fd for the archive, so the first thing is ARMAG. skip those.
    //start read 8 bit in
    lseek(ar, SARMAG, SEEK_SET );
    //then keep reading and printing the headers
    // COMMENTED THE BELLOW OUT
    //struct ar_hdr *bufH = (struct ar_hdr *) malloc(HDSIZE);
    char bufH[HDSIZE];
    //read(ar, bufH, HDSIZE);
    //printf("did read\n");
    int iosize;
    while ((iosize=read(ar,bufH,HDSIZE)) >0 ){
        //printf("item: %s\n", bufH->ar_name);
        //then lseek to skip the contents and get to the next header
        //int skip = atoi(bufH->ar_size);
        //printf("skip : %d\n", skip);
        char  name[16];
        int size = atoi(bufH+16+12+6+6+8);
        //printf("%d", size);
        //int size = atoi(bufH->ar_size) ;
        size = size + size%2; //offsets are on even numbers
        //sprintf(size,"%s", bufH->ar_size);
        //sprintf(name, "%.15s", bufH->ar_name);
        //printf("%.15s\n", bufH->ar_name);
        printf("%.15s\n", bufH);
        //printf("%s \n Size of: %ld", bufH->ar_size, size);
        lseek(ar, size, SEEK_CUR);
    }
}

void tv(int ar){
    //you are given the fd for the archive, so the first thing is ARMAG. skip those.
    //start read 8 bit in
    lseek(ar, SARMAG, SEEK_SET );
    //then keep reading and printing the headers
    // COMMENTED THE BELLOW OUT
    //struct ar_hdr *bufH = (struct ar_hdr *) malloc(HDSIZE);
    char bufH[HDSIZE];
    //read(ar, bufH, HDSIZE);
    //printf("did read\n");
    int iosize;
    while ((iosize=read(ar,bufH,HDSIZE)) >0 ){
        //printf("item: %s\n", bufH->ar_name);
        //then lseek to skip the contents and get to the next header
        //int skip = atoi(bufH->ar_size);
        //printf("skip : %d\n", skip);
        char  name[16];
        int size = atoi(bufH+16+12+6+6+8);
        //printf("%d", size);
        //int size = atoi(bufH->ar_size) ;
        size = size + size%2; //offsets are on even numbers
        //sprintf(size,"%s", bufH->ar_size);
        //sprintf(name, "%.15s", bufH->ar_name);
        //printf("%.15s\n", bufH->ar_name);
        printf("name: %.15s time: %.12s uid: %.6s gid: %.6s  mode: %.8s size: %.10s \n", bufH, bufH+16, bufH+28, bufH +34, bufH+40, bufH+48 );
        //printf("",, bufH+40, 8 10 2 );
        //printf("%s \n Size of: %ld", bufH->ar_size, size);
        lseek(ar, size, SEEK_CUR);
    }
}

void d(int ar, char *filename, char *buffer, int bufS, char** argvs, int len, char *aname  ){
    //you are given the fd for the archive, so the first thing is ARMAG. skip those.
    //start read 8 bit in
    lseek(ar, SARMAG, SEEK_SET );
    //then keep reading and printing the headers
    // COMMENTED THE BELLOW OUT
    //struct ar_hdr *bufH = (struct ar_hdr *) malloc(HDSIZE);
    char bufH[HDSIZE];
    //read(ar, bufH, HDSIZE);
    //printf("did read\n");
    int iosize;
    int dsize = SARMAG; //what has successfully been read
    int size = 0;
    while ((iosize=read(ar,bufH,HDSIZE)) >0 ) {
        //printf("item: %s\n", bufH->ar_name);
        //then lseek to skip the contents and get to the next header
        //int skip = atoi(bufH->ar_size);
        //printf("skip : %d\n", skip);
        char name[16];
        int size = atoi(bufH + 16 + 12 + 6 + 6 + 8);
        //printf("%d", size);
        //int size = atoi(bufH->ar_size) ;
        size = size + size % 2; //offsets are on even numbers
        //you have now read up to this header and size
        dsize += HDSIZE+size;
        //sprintf(size,"%s", bufH->ar_size);
        sprintf(name, "%.15s", bufH);
        //printf("%.15s\n", bufH->ar_name);
        printf("\n%s is found?: ", name);
        int found = inList(argvs, len, name);
        printf("%d\n", found);
        if (found == 1){
            //this means that you don't want to keep that file
            int d2 = dsize;
            dsize -= (HDSIZE + size);
            //printf("s, ds %d %d \n ", d2, dsize);
            //deleting
            ar = del(ar, dsize, bufS, aname, size);
            //
            lseek(ar, dsize, SEEK_SET);
        } else {
            lseek(ar, size, SEEK_CUR);
        }
    }


}

void x(int ar, char *filename, char *buffer, int bufS, char** argvs, int len, char *aname, int restore ){
    //you are given the fd for the archive, so the first thing is ARMAG. skip those.
    //start reading 8 bit in
    lseek(ar, SARMAG, SEEK_SET );
    //then keep reading and printing the headers
    // COMMENTED THE BELLOW OUT
    //struct ar_hdr *bufH = (struct ar_hdr *) malloc(HDSIZE);
    char bufH[HDSIZE];
    //read(ar, bufH, HDSIZE);
    //printf("did read\n");
    int iosize;
    int dsize = SARMAG; //what has successfully been read
    int size = 0;
    while ((iosize=read(ar,bufH,HDSIZE)) >0 ) {
        //printf("item: %s\n", bufH->ar_name);
        //then lseek to skip the contents and get to the next header
        //int skip = atoi(bufH->ar_size);
        //printf("skip : %d\n", skip);
        char name[16];
        int size = atoi(bufH + 16 + 12 + 6 + 6 + 8);
        //printf("%d", size);
        //int size = atoi(bufH->ar_size) ;
        size = size + size % 2; //offsets are on even numbers
        //you have now read up to this header and size
        dsize += HDSIZE+size;
        //sprintf(size,"%s", bufH->ar_size);
        sprintf(name, "%.15s", bufH);
        //printf("%.15s\n", bufH->ar_name);
        printf("\n%s is found?: ", name);
        int found = inList(argvs, len, name);
        printf("%d\n", found);
        if (found == 1){
            //this means that you want to extract that file
            int d2 = dsize;
            dsize -= (HDSIZE + size);
            int fper = atoi(bufH + 16 + 12 + 6 + 6);
            int fsi = atoi(bufH + 16 + 12 + 6 + 6 + 8);
            long time = atoi(bufH + 16 );
            //printf("%d %d \n", fper, fsi);
            //printf("%ld\n", time);
            int gid = atoi(bufH + 16 + 12 + 6);
            int uid = atoi(bufH + 16 + 12 );
            //all ar have / after name
            name[strlen(name) - 1] = '\0';
            //if (strcmp("/",  )){
                //name[strlen(name)-1] = " ";
            //}
            // printf("%s %d %d %d ", name, uid, gid,strlen(name) );
            ext(ar, bufS, name, fper, fsi,restore, time, gid, uid);
            //ext(ar, bufS, name,fper, fsi, restore, time);
            lseek(ar, fsi%2, SEEK_CUR);
            //lseek(ar, size%2, SEEK_CUR);
        } else {
            lseek(ar, size, SEEK_CUR);
        }
    }


}







// based on copy.c in share

int main (int argc, char** argv)
{
    int fd1,fd2;		/* file descriptors */
    //char buff [4096];   /* overkill on buffer size*/
    int size;			/* real buffer size */
    int iosize;			/* actual amount read */

    // the arguments in are flag archive file
    // below is debugging stuff
    if (argc < 3){
        printf("too few args, give myar ACTION ARCHIVE FILE+\n");
        return (2);
    }
    //printf(argv[0]);
    //printf(argv[1]);
    //printf("%s", argv[2]);





    struct ar_hdr *hd =  (  struct ar_hdr * ) malloc(sizeof(HDSIZE));


    fill_ar_hdr(argv[3], hd);

    struct meta *m = (  struct meta * ) malloc(sizeof(MSIZE));

    fill_meta(hd, m);

    printf("\nEnd debugging, doing comand:\n\n");
    //as all of the action require that there be an archive, go ahead and open a file descriptor with the option to create if it
    //doesnt exist
    int arfd;
    //flags are set so it is read and write, is  appeneded to if new info is written, and if not existant created with 0666 permission
    arfd = open(argv[2], O_RDWR|O_APPEND);
    //then, if it dose not exist, create the file and write the ARMAG to it
    if (arfd==-1){
        printf("\narchive %s doesn't exist :(\n", argv[2]);
        if (strcmp(argv[1], "q")==0){
            printf("Creating!\n");
            arfd = open(argv[2],O_RDWR | O_CREAT |O_APPEND,0666);
            //because writes are by way of the buffer, write the magic number to the buffer
            //actually simpler not to, since this something that woul happen at most once per my ar, just go ahead and do the write system call
            write(arfd, ARMAG, SARMAG);
        } else { return 0;}

    } else{
        //make sure it is an AR file
        char mag[8];
        read(arfd,mag, SARMAG-1);
        //printf("%s", mag);
        if (strcmp(mag, "!<arch>") != 0){
            printf("\n %s not an archive!\n", mag);
            //return (-1);
        }
    }



    //there is now a fd to the archive, as only t, tv and A (for now for A) do not need a fd to the file to add, do those first
    if (strcmp(argv[1], "t")==0){
        // print contents (short)
        printf("\nthe table of contents:\n");
        t(arfd);
        close(arfd);
        return(0);
    }


    if (strcmp(argv[1], "tv")==0){
        printf("The verbose table of contents:\n");
        tv(arfd);
        close(arfd);
        return(0);
    }

    //All of the other comands need a buffer
    //stat to get the blocksize
    struct stat fileStat;
    stat(argv[2], &fileStat);
    //printf("\n%d", (int)fileStat.st_blksize);
    //because ar only does one option at a time, there is a single buffer in the ain loop instead of a buffer per function
    int Bsize = (int)fileStat.st_blksize;
    char buff[Bsize];


    if (strcmp(argv[1], "A")==0){
        // Append all regular files but self in curent directory
        //loop to list files based on https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
        struct dirent *de;  // Pointer for directory entry

        // opendir() returns a pointer of DIR type.
        DIR *dr = opendir(".");

        if (dr == NULL)  // opendir returns NULL if couldn't open directory
        {
            printf("Could not open current directory" );
            return 0;
        }

        // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
        // for readdir()
        while ((de = readdir(dr)) != NULL) {
            //no file has an over 15 char name
            char file[16];
            sprintf(file,"%.15s", de->d_name);
            //printf("%s\n", file);

            struct stat fStat;
            stat(file, &fStat);
            if (S_ISREG(fStat.st_mode)){
                //printf("%s is regular\n", file);
                //make sure not too append yourself
                if(strcmp(file, argv[2])!=0){
                    append(arfd, file, buff, Bsize);
                    printf("is the permission of %s . File is being appended to %s\n", file, argv[2]);

                }
            }
        }
        //close dir gives an error that I am trying to free more mem than I can
        //free(): invalid next size (normal)
        // closedir(dr);
        close(arfd);
        return(0);
    }


    if (strcmp(argv[1], "d")==0) {
        //to delete, the there is no need to verify the file, just pass along with ar anf buffer
        //inorder to not do multiple deletes without multiple passes, pass the arg of the files to delete
        //printf("%d", argc-3);
        if (argc-3<1){
            printf("No files to delete \n");
            close(arfd);
            return 1;
        } else {
            //other wise remove sending the list of files to remove
            int numD = argc-3;
            char* kill[numD];
            int i = 0;
            while (i < numD){
                kill[i] = argv[i+3];
                //printf("%s", kill[i]);
                i++;
            }
            //printf("%d", inList(kill, numD, "ttxt"));
            d(arfd, argv[3], buff, Bsize, kill, numD, argv[2]);
            close(arfd);
        }

    }


    // to extract files
    if (strcmp(argv[1], "x")==0) {
        //to delete, the there is no need to verify the file, just pass along with ar anf buffer
        //inorder to not do multiple deletes without multiple passes, pass the arg of the files to delete
        //printf("%d", argc-3);
        if (argc-3<1){
            printf("No files to extract \n");
            close(arfd);
            return 1;
        } else {
            //other wise remove sending the list of files to remove
            int numD = argc-3;
            char* ex[numD];
            int i = 0;
            while (i < numD){
                ex[i] = argv[i+3];
                //printf("%s", kill[i]);
                i++;
            }
            //printf("%d", inList(kill, numD, "ttxt"));
            x(arfd, argv[3], buff, Bsize, ex, numD, argv[2], 0);
            close(arfd);
        }

    }

    // to extract files and restore time
    if (strcmp(argv[1], "xo")==0) {
        //to delete, the there is no need to verify the file, just pass along with ar anf buffer
        //inorder to not do multiple deletes without multiple passes, pass the arg of the files to delete
        //printf("%d", argc-3);
        if (argc-3<1){
            printf("No files to extract \n");
            close(arfd);
            return 1;
        } else {
            //other wise remove sending the list of files to remove
            int numD = argc-3;
            char* ex[numD];
            int i = 0;
            while (i < numD){
                ex[i] = argv[i+3];
                //printf("%s", kill[i]);
                i++;
            }
            //printf("%d", inList(kill, numD, "ttxt"));
            x(arfd, argv[3], buff, Bsize, ex, numD, argv[2], 1);
            close(arfd);
        }

    }





    //for all others loop though the remaining argv doing the action specified and do it for the "extra" argv
    int i = 3;
    while (i<argc ){
        //printf("%s", argv[i]);
        if (strcmp(argv[1], "q")==0) {
            // in the case of q, each argv[i] must exist and be a regular file
            struct stat fStat;
            stat(argv[i], &fStat);
            if (S_ISREG(fStat.st_mode)){
                //printf("%s is regular\n", file);
                //make sure not to append yourself
                if(strcmp(argv[i], argv[2])!=0) {
                    append(arfd, argv[i], buff, Bsize);
                    printf("is the persmission of %s . File is being appended to %s\n", argv[i], argv[2]);
                }
                }
            //printf("q");
        }


        i++;

    }
    // once all the argv are done, close the fd
    close(arfd);




    //for this assignment, the archive will be all text

    /*
    if ((fd1=open(argv[1],0)) == -1) {
        perror("open1");
        exit(1);
    }

    if ((fd2=open(argv[2],O_WRONLY | O_CREAT |O_TRUNC,0644)) == -1) {
        perror("open2");
        exit(1);
    }

    size=atoi(argv[3]);



    while ((iosize=read(fd1,buff,size)) >0 )
        write(fd2,buff,iosize);

    if (iosize == -1) {
        perror("read");
        exit(1);
    }
*/
}