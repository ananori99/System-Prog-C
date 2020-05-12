//this code is my own work written without a tutor or other students - Ana Noriega

	
#include <stdlib.h>
#include <stdio.h>
#include <math.h> //for ceiling


typedef struct _seg {  /* definition of new type "seg"*/
    int  bits[256];
    struct _seg  *next;
}seg  ;

#define BITSPERSEG  (256*sizeof(int)) //this used to be 8*256*sizeof(int)

/*"bitmap will be represented as a doubly linked list of segments each containing 8192 bits.
  Defined by structure */



typedef struct {    //
	seg *segpt;
	int intnum;
	int bitnum;
	} coordinate;

//so that  whichseg can use these
seg *head,*pt;
//I really doubt this is the "right" solution but it is a way for me to deal with whichseg
int curentSeg, incPtr;

//the 9 functions we must write
//THIS GOT CHANGED AWAY FROM seg*
seg* whichseg(int j){ //calc ptr to seg containing bit for j.
    //I CHANGED AWAY FROM *seg BECAUSE THE COMPILER SAID TO
    int k = j/BITSPERSEG+1;//find the segment number that it should be in. Then treat as pointer by defining intnum and bitnum
    //should pt be a global that I modify???
    //or access head and go to next k times?
    seg* S = head;
    for (int i=1;i<k;i++) { //Just Forward Links for Now
        S = S->next;
    }
    curentSeg = k; //maybe???
    return S; //this should be how many hops ahead to take
	}

int whichint(int j){ //the index of the int in the array BITS
    return (j%BITSPERSEG)/32;
	}

int whichbit(int j){ //the position of the bit in the integer array
    return j%32;
	}

coordinate getcoord(int j){ //finds the coordinate a int should be at
    coordinate C;
    C.bitnum = whichbit(j);
    C.intnum = whichint(j);
    C.segpt = whichseg(j);
    return C;
	}

void markcoord(coordinate c){ //set the bit to 1 if it is not prime
    //get the pointer
    int b = c.bitnum;
    int i = c.intnum;
    seg* s = c.segpt;
    //then the int at that bit will be ORed with 00...1..0, where the location of this 1 depends on what bit coordinate says to change
    //first, create this int that will be used for the OR
    int k;
    //then shift it according to the coordinate
    k = 1 << b;
    //then go to the right segment, and the right INT
    int INT = s->bits[i];
    //then or this int with the shifted one
    INT = INT|k;
    //set this int INT to be the one stored in the bitmap
    s->bits[i] = INT;
	}

int testcoord(coordinate c){ //return 0 if prime, 1 if not prime
    //get the pointer
    int b = c.bitnum;
    int i = c.intnum;
    seg* s = c.segpt;
    //then the int at that bit will be ANDed with 00...1..0, where the location of this 1 depends on what bit coordinate says to change
    //first, create this int that will be used for the OR
    int k;
    //then shift it according to the coordinate
    k = 1 << b;
    //k = 0;
    //then go to the right segment, and the right INT
    int INT = s->bits[i];
    //then AND this int with the shifted one
    _Bool t = INT & k;
    return (int) t;
}


void marknonprime(int j){
    //use getcoord to get the coordinate
    coordinate C = getcoord(j);
    //use markcoord to mark nonprime
    markcoord(C);
}


int testprime(int j){
    //get the coordinate
    coordinate C = getcoord(j);
    //test the coordinate
    return  testcoord(C);
}

//It seems like this might not be updating the actual pointer
coordinate incrcoord(coordinate c, int inc){//increment the coordinate by inc
    //then if you are moving ahead by inc, see if you need to change *seg, int or bit
    //increment is in terms of bits, so see if that causes "overflow" into the next integer
    int new = c.bitnum + inc;
    //the new bit you are at
    c.bitnum = new%32;
    //if you need to change the integer you are in
    int newS = new/32;
    //if this is not 0, then this is the number of ints to hop forward
    if (newS){
        //are you still in the same segment? ie are the present int plus the hop forward still within the 256 int array?
        int t = newS + c.intnum;
        c.intnum = t%256;
        if (t>=256){ //in case the new int you are at "overflows" the segment
            //do the appropriate number of hops forward
            printf("I am hopping a segment");
            for (int i = 1; i < ((int) (t/256)); i++) {
                seg* nex  = c.segpt->next;
                //do I need to change the current seg indicator int???
                //catch if that segment dosen't exist
                if (nex == NULL) { break;}
                else { c.segpt = nex;
                incPtr++;
                }
            }
        }
    }
    incPtr += curentSeg;
    return c;
}

int whichnum(coordinate c){ //to recover the original j from a coordinate
    int n = 0;
    //number of segments times BITSPERSEG - BITSPERSEG
    n += (curentSeg-1) * BITSPERSEG;

    //plus INT times 32
    // -1 because if I
    n += c.intnum*32;

    //plus bit
    n += c.bitnum;

    //then realize this is the number of odds
    n = 2*n +1;
    return n;
}



void main(int argc, char *argv[]) {


    int	 i;
    int howmany;



    if (argc == 2) sscanf(argv[1],"%d",&howmany);
    else scanf("%d",&howmany);
    //howmany = (howmany +BITSPERSEG -1)/BITSPERSEG; //old one, changed because of the odd only storage
    int j = howmany;

    //as you only need to store odds?
    //the number segments to allocate so that odd numbers between 1 and N can be represented by a bit.
    int numOdds = ceil(howmany/2);
    howmany = (numOdds +BITSPERSEG -1)/BITSPERSEG;

    head= (  seg * ) malloc(sizeof(seg));
    pt=head;
    for (i=1;i<howmany;i++) { //Just Forward Links for Now
        pt->next = (  seg *) malloc(sizeof (seg));
        pt=pt->next;
    }

    printf("Done allocating %d nodes\n",i);
    //this ends the given code for allocating nodes... as you only need to store odds, modify this?


    /*
    //testing the 9 functions
    printf("which bit: %d \n" ,  whichbit(7));
    printf("which int: %d \n" ,  whichint(7));
    printf("which seg: %d , %d \n" ,  whichseg(7), curentSeg);
    printf("there are now enough segments to store the %d odds \n", numOdds ); //obvoiusly do not store and check multiples of 1
    coordinate cor = getcoord(7);
    coordinate tescor = getcoord(5);
    printf("getcord:  %d , %d, %d\n", curentSeg, cor.intnum, cor.bitnum);
    printf("whichnum: %d, cor: %d %d %d, seg add: %d\n", whichnum(cor), curentSeg, cor.intnum, cor.bitnum, cor.segpt);
    printf("testcoord: %d\n", testcoord(cor));
    markcoord(cor);
    printf("did markcoord, 0 means prime\n");
    printf("testcoord for changed: %d\n", testcoord(cor));
    printf("testcoord for unchanged: %d\n", testcoord(tescor));
    printf("done\n");
    printf("testprime: %d\n", testprime(numOdds) );
    printf("testprime: %d\n", testprime(1) );
    marknonprime(numOdds);
    printf("did marknonprime, 0 means prime\n");
    printf("testprime: %d\n", testprime(numOdds) );
    printf("testprime on unchanged: %d\n", testprime(1) );
    printf("whichnum: %d, cor: %d %d %d, seg add: %d\n", whichnum(cor), incPtr, cor.intnum, cor.bitnum, cor.segpt);
    cor = incrcoord(cor, 10000);
    printf("increased coor\n");
    printf("whichnum: %d, cor: %d %d %d, seg add: %d\ntrying get coord: \n", whichnum(cor), incPtr, cor.intnum, cor.bitnum, cor.segpt); */

    //all bits are initially zero, as in prime, and the seive is then applied in order to remove all multiples of primes.
    //So, start at 3 you check: 3 5 7 9 11 13 ... 25 ...
    //for 3, skip ahead 3 and those are the multiples: 3 .. 9.. 15 .. 21.. 27
    //for 5, skip ahead 5: 5 ....15 .... 25
    //for 7, skip ahead 7: 7 ...... 21 ...... 21
    //so the loop is that up to sqrt n you check each odd for being prime, and if prime you mark out it's multiples

    //YOU NEED TO ADJUST THIS TO BE FOR SQRT N

    //form how the helpers work, int j is mapped to 2*j+1:
    //so 1, 2, 3, 4 -> 3, 5, 7, 9
    //and than because inccoord hops ahead in the array, it goes two ahead each time.
    //ie your number line is the line of odds
    //ie incrcoord(7,1) -> 9
    printf("primes found: \n");


    for ( i = 1; i < numOdds; i++){
        int odd = i*2+1;
        //printf("just checking odds: %d\n", odd);
        //then go through checking and marking each odd for being prime
        //first, for that odd get the coordinate
        //I think so far the logic of getcoord is from the point of veiw of the array in memory, not of the number line...
        //so the input j is the jth bit, not integer j
        //coordinate cordW = getcoord(i);
        //printf("num from coord: %d\n",whichnum(cordW));
        //see if it is prime, and if so mark all the multiples.
        //testcoord returns zero if prime

        if (!testprime(i)){
            printf("  %d ",odd);
            //the odd multiples are all the prime incraments forward...
            //ie 3 -> 6, 9, 12, 15 (filter odd) -> 9, 15-> 1*2+1 -> 4*2+1, 7*2+1....
            //I am only checking up to rtJ as in the final sum, the number of 0 up to rtJ is the total primes...
            //this is the same as a fold of all 1s subtracted from rtJ???
            int hop = i*2+1;
            //printf("I will hop ahead by %d\n", hop);
            for (int k = i+hop; k < numOdds; k = k+hop){
                marknonprime(k);
                //printf("I marked the %d bit nonprime. This is number %d\n", k, 2*k+1);
            }
        }




    }

    printf("\nI finished building my bitmap, and will now do an accumulation loop with shifts to see how many 0s I have.\n");

    //once this is done, the bit map is fully built
    //so just pass through it counting the zero bits
    //so for each segment, and each array, and each int

    //an accumulator
    int c = 0; // to accumulate the bit count of zeros. The notation 2*j+1 starts at 3, so start at 1 to account for 2
    //printf("int accum %d", c);


    pt = head;
    while ( pt != NULL ){ //until there are no segments
        for (int f = 0; f < 256; f++){ //for each int in the array bits
            int k = pt->bits[f];
            int p = 0;
            for (int n = 0; n < 32; n++) //count all bits that say prime by shifting so they are in the position
            {
                k >>= 1;
                if((k & 1)){//printf("bitmap has prime\n");
                //printf("current num %d\n", f*32+n);
                p++;}
            }
            c += p;
        }
        pt = pt->next;
        if (pt != NULL) {
            pt->next = pt->next->next;
        }
    }
    c = numOdds - c; //as 1 stands for not prime, and we know that the primes are odd (plus 2)

    printf("I have %d primes total!\nNow to find triple primes...\n", c);

    //read in a pair of even integers
    printf("I will now read from stdin to get the even integers K and M, these are the increments for the triple prime...\nIf they are not even, they will be truncated to be even and positive.");
    int M, K;
    //while not EOF
   // scanf("%d  %d", &K, &M);  //reads stdinput


    //_Bool true = 0;

    while((scanf("%d %d", &K, &M)==2)) {

        //to make using incrament easier, I want to store my increment numbers as the "steps" forward that are to be taken on the bitmap, from least to greatest.
        //ie, user says 6,4 so: 4,6 -> 4,6-4 -> 2,1
        //that way if I am true at p+4, I only do one more hop to check p+6...
        M = abs(M);
        K = abs(K);
        if (M < K) {
            int t = K;
            K = M;
            M = t;
        }

        printf("your K,M  %d,%d are stored as the \"bit-steps\" forward ", K, M);
        M = M / 2;
        K = K / 2;
        printf("%d, %d. This is K = %d, M = %d \n ", K, M, K * 2, M * 2);
        M = M - K;


        //only check up to the sqrt??? This seems to be wrong...
        //int rtJ = sqrt(j);
        //printf("root of %d, so only check up to here : %d \n", j, rtJ);
        //int max = rtJ/2 +1;
        //printf("max number of bits to check for loops: %d\n", max);

        //triple prime part should be writen with testcoord and incrcoord,
        //it should print the total number of triple primes and the largest ones found
        printf("finding triple primes...\n");
        //an accumulator
        int tc = 0; // to accumulate the bit count of zeros. The notation 2*j+1 starts at 3, so start at 1 to account for 2
        //printf("int accum %d", c);


        pt = head;
        coordinate tcoord = getcoord(0);
        int bigest = 0;
        //printf("what? %d", whichnum(tcoord));
        while (pt != NULL) { //until there are no segments
            for (int i = 0; i <= numOdds - 1 - K - M; i++) { //for each number, check if it is a triple prime
                if (!testcoord(tcoord)) {//if that coordinate is prime
                    //check the coordinate K steps ahead, to see if it is prime...
                    coordinate tcoordK = incrcoord(tcoord, K);
                    coordinate tcoordM = incrcoord(tcoordK, M);
                    if ((!testcoord(tcoordK) && !testcoord(tcoordM))) {//if P+K is prime, check P+M
                        tc++;
                        //printf("a triple prime: %d, %d , %d    ", whichnum(tcoord), whichnum(tcoordK), whichnum(tcoordM));
                        bigest = whichnum(tcoord);//if P,P+K, and P+M are all prime. count it and save save biggest
                    }

                } //now go on to check the next bit!
                tcoord = incrcoord(tcoord, 1);
            }
            pt = pt->next;
            if (pt != NULL) {
                pt->next = pt->next->next;
            }
        }


        printf("There are %d triple primes \nthe largest is %d,%d,%d\ngive new K and M \n", tc, bigest, bigest + 2 * K,
               bigest + 2 * (M + K));

        //_Bool true = scanf("%d  %d", &K, &M) != EOF;  //reads stdinput
        //printf("EOF? %d", true);

    }

}



