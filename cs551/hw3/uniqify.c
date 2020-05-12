//this code is my own work written without an outside tutor or code by other students - Ana Noriega

/* BASED ON pipes.c, program to illustrate process creation via fork
	    and exec
	    Uniqify should read a text file and output the unique wprds in the file,
	    sorted alphabetically.
	    Input from std in, output std out ( WORD #SEEN \n )

	    Internally, has 3 types of processes.
	    1 reads input, parsing lines to words and giving to sorters round robin.
	    N do the sorting, N from command line.
	    1 supresses and counts duplicate words and writes output.

        ALL PARENTS MUST WAIT ON CHILDREN
        USE THE SYSTEM SORT COMMAND WITH NO ARGS
        YOUR PROG STARTS THE PROCESS AND PLUMBS THE PIPES
        USE FGETS, FPUTS, FDOPEN, FDCLOSE
        PARSER CONVERTS ALL TO LOWER CASE, NON ALPHABET ARE DELIMITERS AND ARE DISCARDED
        WORDS LESS THAN 5 CHAR DISCARDED, TRUNCATE OVER LEN(W) = 25*/

#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#define wordLen 25




// just not to have loops everywhere
void closeAr(int **pipe, int n)
{
    int i, j;
    for (i = 0; i < n; i++) {
        for ( j = 0; j < 2; j++) {
            close(pipe[i][j]);
        }
    }
}

//parsing lines to words by fgets and fputs
//the text, number of sorts, array of pipes
void  parser( int n, int *pipes){
    // open up the pipes
    FILE *outP[n];
    for (int i = 0; i < n; i++) {
        outP[i] = fdopen(pipes[i*2+1], "w");
    }

    char word[25]; //to truncate the words sent
    //THE INPUT IS FROM STDIN, so scanf gets word at a time and returns # read or EOF
    int eoff =  scanf("%*[^a-zA-Z]");
    int k= 0;
    while (eoff != EOF){
        eoff = scanf("%[a-zA-Z]", word);
        // from https://stackoverflow.com/questions/2661766/how-do-i-lowercase-a-string-in-c
        for(int i = 0; word[i]; i++){
            word[i] = tolower(word[i]);
        }
        fputs(word,outP[k%n]);
        //sort wants new line
        fputs("\n",outP[k%n]);
        printf("%s\n", word);
        word[0] = '\0';
        eoff = scanf("%*[^a-zA-Z]");
        k++;
    }
    //when you are done, flush out and close the no longer needed pipes
    for (k =0; k < n; k++){
        fclose(outP[k]);
    }
}

// with https://stackoverflow.com/questions/33532570/c-parsing-input-text-file-into-words
int splitInput( int n, int *pipes ) {
    int i = 0;
    char *cp;
    char *bp;
    char line[255];
    char *array[500];
    int x;
    char word[25]; // to truncate
    // open up the pipes
    FILE *outP[n];
    for (int i = 0; i < n; i++) {
        outP[i] = fdopen(pipes[i*2+1], "w");
    }
    // read a line with fgets, from stdin
    while (fgets(line, sizeof(line), stdin) != NULL) {
        int k = 0;
        bp = line;
        while (1) { // still processing the line
            cp = strtok(bp, "[^a-zA-Z],.!? \n");
            bp = NULL;

            if (cp == NULL)
                break;
            strcpy(word, cp);
            //word = cp;
            // the tokens are now in array
            printf("Check print - word %i:%s:\n",i-1, cp);
            // turn to lower case
            // from https://stackoverflow.com/questions/2661766/how-do-i-lowercase-a-string-in-c
            for(int l = 0; word[1]; i++){
                word[l] = tolower(word[l]);
            }
            // hand word to the next sorter
            fputs(word,outP[k%n]);
            //sort wants new line
            fputs("\n",outP[k%n]);
            printf("%s\n", word);
            word[0] = '\0'; //empty word
            k++;
        }

    }
    // have read all std in
    // close up pipes so EOF
    for (int k =0; k < 2*n; k++){
        fclose(outP[k]);
    }
    return 0;
}

//just to not have loops everywhere
void closeP(int *arr, int num){
    for (int j = 0; j < 2*num; j++){
        close(arr[j]);
    }
}

void parse2(int **pipes, int n)
{
    // open up the pipes
    FILE *out[n];
    for (int i = 0; i < n; i++) {
        out[i] = fdopen(pipes[i][1], "w");
    }

    char word[25]; //to truncate the words sent
    //THE INPUT IS FROM STDIN, so scanf gets word at a time and returns # read or EOF
    int eoff =  scanf("%*[^a-zA-Z]");
    int k= 0;
    while (eoff != EOF){
        eoff = scanf("%[a-zA-Z]", word);
        // from https://stackoverflow.com/questions/2661766/how-do-i-lowercase-a-string-in-c
        for(int i = 0; word[i]; i++){
            word[i] = tolower(word[i]);
        }
        if (strlen(word)>4) {
            fputs(word, out[k % n]);
            //sort wants a new line
            fputs("\n", out[k % n]);
        }
        word[0] = '\0';
        eoff = scanf("%*[^a-zA-Z]");
        k++;
    }
    //when you are done, flush out and close the no longer needed pipes
    for (k =0; k < n; k++){
        fclose(out[k]);
    }


}

// refrence to https://stackoverflow.com/questions/30808565/counting-word-occurrences-in-a-file-c

// using counter...
// refrence:
// https://stackoverflow.com/questions/21865041/in-c-what-exactly-happens-when-you-pass-a-null-pointer-to-strcmp
// https://www.geeksforgeeks.org/frequency-substring-string/
// https://codeforwin.org/2016/04/c-program-to-search-all-occurrences-of-word-in-string.html
// https://www.geeksforgeeks.org/count-occurrences-of-a-word-in-string/
// https://codeforwin.org/2016/04/c-program-to-count-occurrences-of-word-in-string.html
// https://javatutoring.com/c-program-count-occurrences-of-a-word-in-string/
// https://codeforwin.org/2018/02/c-program-count-occurrences-of-a-word-in-file.html
int suppres(int **in_pipe, int n)
{
    int i, j,k;
    char **words;
    FILE *sort[n];
    int new_words = 0;
    int nwords = 0;
    typedef struct element {
        char word[25];
        int how_many;
    } element;
    // do not cast what malloc returns. Also struct is not needed.
    element * dictionary = malloc(sizeof (element) * nwords);
    struct element *cur_word = malloc(sizeof(struct element));
    int null_count = 0; //Increments if output from pipe is NULL (meaning EOF)
    // where to grab the top words into
    words = malloc(n * sizeof(char *));
    for (i = 0; i < n; i++) {
        words[i] = malloc(wordLen * sizeof(char));
    }



    int is_word_new(element *dictionary, char *string) {
        int i;
        for (i = 0; i < new_words; i++) {
            printf("|%s|, |%s|\n", string, dictionary[i].word);
            if (strcmp(string, dictionary[i].word) == 0)
                return 0;
            printf("i=%d\n", i);
        }
        return 1;
    }

    int which_word(element **dictionary, char *string) {
        int i;
        for (i = 0; i < new_words; i++) {
            if (strcmp(string, dictionary[i]->word) == 0)
                return i;
        }
        return 0;
    }

    //to select which of the sorted words to compare at
    int least(int n, char **words)
    {
        int least = -1;
        int i;
        // find a word
        for (i = 0; i < n; i++) {
            if (words[i] != NULL) {
                least = i;
                break;
            }
        }
        if (least == -1)
            return -1;
        /* Now find the lowest alphabetical word */
        for (i = 0; i < n; i++) {
            if (strcmp(words[i], words[least]) < 0){
                if(words[i] != NULL){
                    least = i;
                }
            }
        }
        return least;
    }
    // open the pipes, and see if any are empty
    for (i = 0; i < n; i++) {
        sort[i] = fdopen(in_pipe[i][0], "r");
        if (fgets(words[i], wordLen, sort[i]) == NULL){
            words[i] = NULL;
            null_count++;
        }
    }
    //stat printing from the first to the last
    k = least(n, words);
    strncpy(cur_word->word, words[k], wordLen);
    cur_word->how_many = 1;

    while (null_count < n) {
        if (fgets(words[k], wordLen, sort[k]) == NULL) {
            words[k] = NULL;
            null_count++;
        }
        k = least(n, words);
        if (k == -1) //you are done
            break;
        // in case on the same word
        if (strcmp(cur_word->word, words[k])==0) {
            cur_word->how_many++;
        } else {
            // if done with that word
            printf("%d %s", cur_word->how_many, cur_word->word);
            strncpy(cur_word->word, words[k], wordLen);
            cur_word->how_many = 1;
        }
    }

    printf("%5d %s", cur_word->how_many, cur_word->word);

    for (i = 0; i < n; i++) {
        free(words[i]);
    }
    free(words);
    free(cur_word);

    for (i = 0; i < n; i++) {
        fclose(sort[i]);
    }

    return 0;
}


/* removes trailing newline characters from strings */

void free_pipes_array(int num_pipes, int **pipes_array)
{
    int i;
    for (i = 0; i < num_pipes; i++) {
        free(pipes_array[i]);
    }
    free(pipes_array);
}
















int main(int argc, char **argv) {
    int i; /* generic counter */
    //int pid[2];                /*pids of children */
    int whom;                /* pid of dead child*/
    int esize;                /* number of env variables */
    char **newenvp;         /* constructed env pointer */
    int status;                /* childs return status*/
    int fin, fout;

    if (argc < 2) {
        printf("too few args, give number of sorters\n");
        return (2);
    }

    //then as each process is doing diffrent jobs, no need for shared var beyond pipes. so fork
    int children = atoi(argv[1]);
    // if the arg was no an int
    if (children == 0) {
        printf("argv[1] is not a valid # of processes \n");
        return 0;
    }

    // then, set up an array for pipes
    // remeber that each call to pipe returns two fd in the form [0][1]
    // so pipe array is [[0][1]],[[0],[1]]...
    int **parseP = malloc(sizeof(int *) * (children));
    for (int i = 0; i < children; i++) {
        parseP[i] = malloc(sizeof(int) * 2);
    }
    int **supP = malloc(sizeof(int *) * (children));
    for (int i = 0; i < children; i++) {
        supP[i] = malloc(sizeof(int) * 2);
    }

    //now there is room for all the pipes, parent needs all parse pipes, each sorter only needs it's n index pipe
    //Spawn all the sort processes
    pid_t pid;
    for (i = 0; i < children; i++) {
        if (pipe(parseP[i]) == -1) {
            printf("error on pipe");
            exit(10);
        }
        if (pipe(supP[i]) == -1) {
            printf("error on pipe");
            exit(10);
        }

        if ((pid = fork()) == -1) {
            printf("error on fork\n");
            exit(i + 1);
        }

        switch (pid) {
            case 0:
                /* this is the child */
                // the child reads from the read end of parseP
                // writes to the write end of supP
                /* we will redirect stdout */

                // fdopen according to https://github.com/angrave/SystemProgramming/wiki/Pipes,-Part-2:-Pipe-programming-secrets
                // http://c.happycodings.com/gnu-linux/code25.html
                // https://www.gnu.org/software/libc/manual/html_node/Creating-a-Pipe.html
                close(1);
                FILE *stream = fdopen (supP[i][1], "w");
                dup2(stream,1);
                if (dup2(supP[i][1], 1) ==
                    -1){
                    printf("dup");
                    exit(-1);}
                /* close the side of pipe I am not going to use */
                close(supP[i][0]); //read of outward


                /* we will redirect stdin */
                close(0);
                FILE *stream2 = fdopen(parseP[i][0],"r");
                dup2(stream2,0);
                if (dup2(parseP[i][0], 0) ==
                    -1){ exit(-10);}

                // the younger siblings' pipes are still open, causing no eof
                closeAr(parseP, (i + 1));
                closeAr(supP, (i + 1));
                // now, the child has redirected in/out
                // do sort, lp for command line and path
                execlp("sort", "sort", (char *)NULL);
                //after this the children just run sort code
            default: //else { //the parent
                //close parseP reads
                close(parseP[i][0]);
                //close supP writes
                close(supP[i][1]);
                //stop looping so it's not n*n
                break;
        }
    }

    // now all children exist
    //as the supressor does not yet exist don't close everything yet-
    //parent needs the parseP writes, the sup needs the supP reads
    parse2(parseP, children);
    //the parent can close all supP
    //the sup can close all parseP

    //create the sup
    if ((pid=fork())==-1){
        printf("error in sup fork\n");
        exit(-15);
    }
    if (pid == 0 ){
        //the child/ supressor
        //close all parseP, as only parser and sorters use these
        //closeP(parseP, 2*children);
        suppres(supP, children);
        exit(1);
     }
    //the parent
    //close unused pipes
    //all supP
    //closeP(supP, 2*children);


    //wait on all children, and suppressor
    for (i = 0; i < children+1; i++) {
        wait(NULL);
    }
    //all done, no need for pipes
    for (i = 0; i < children; i++) {
        free(parseP[i]);
        free(supP[i]);
    }
}

