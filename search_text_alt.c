
// This program ATTEMPTS to search a string using a multiprocessor approach.
//
// BUILD INSTRUCTIONS - mpicc -Wall -o <object name> <file-name>.c
//                      mpirun -np <# procs> <object name> <search string> <text file>


#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

//Traverse a string and remove any instances of rmChar
//params: str, a string passed by pointer
//params: rmChar, a character for which all instances will be removed from str
//pre: str has all instances of rmChar
//post: all instances of rmChar are removed from str and unreplaced.
//no returns
void delChar(char * str, char rmChar) {
    char *tmp1, *tmp2;

    for(tmp1 = tmp2 = str; *tmp2 != '\0'; ++tmp2) {
        *tmp1 = *tmp2;
        if(rmChar != *tmp1)
            ++tmp1;
    }
    *tmp1 = '\0';
}

//Parse a string, removing all instances of beginning and ending quotes and escape slashes
//params: str, a string
//pre: string from the command line that may or may not have escape back-slashes and quotes
//post: all \, starting/ending " and starting/ending ' are removed
//returns a pointer to a string
char * stringParse(char *str) {
    char * z = str;

    if ('"' == *z){
        delChar(z, '"');
        delChar(z, '\\');
    }
    else if ('\'' == *z)
        delChar(z, '\'');

    return z;
}

//Traverses a text string, searching for all instances of search
//params: search, a string that is sought in the 'text'
//params: text, a string that is used as a search field for 'search'
//params: offset, the min index of the current processors text portion.
//pre: no instances of the 'search' key are located in the 'text' string
//post: any instances of 'search' that exist in 'text' are indexed in
// the 'indexes' array, where indexes are adjusted by the offset to gives
// the true value of the index in the text as a whole
int *scanText(char *search, char *text, long offset) {
    int arrayIndex = 0;
    int *indexes = malloc(strlen(text) * sizeof(int));

    for(int i = 0; i < strlen(text); ++i){
        int iCpy = i;
        for(int j = 0; j < strlen(search); ++j){
            //check if the search string continues to match the current text
            //printf("search:text - %c : %c\n", search[j], text[iCpy]);
            if(search[j] == text[iCpy]){
                //if reaching the end of the search string with a match
                //add the index of the text where the match started to
                //the indexes array
                //printf("j:sizeofsearch - %i : %i \n", j, sizeof(search));
                if(j == strlen(search) - 1){
                    indexes[arrayIndex] = i + offset;
                    arrayIndex+=1;
                }
                iCpy++;
            }
            else
                break;
        }
    }
    return indexes;
}

int main(int argc, char* argv[])
{
    if(argc < 2){
        printf("Too few arguments. Exiting.");
        exit(1);
    }

    struct stat statbuff;
    int id; //processor id
    int p; //num processors
    char *search, *localText;
    long locMin, locMax, min, max; //hold the indexes of the text sent to each processor
    int localElems;
    //int prompt;

    if(stat(argv[2], &statbuff) == -1) {
        printf("Could not stat the file %s. Exiting\n", argv[2]);
        exit(1);
    }

    long n = (long)statbuff.st_size; //holds the size of the file in bytes

    search = stringParse(argv[1]); //remove any escape characters from the string
    int searchLen = sizeof(search); //holds the length of the search string, used for overlap

    if(searchLen > n){
        printf("Search string is larger than the text file. Exiting");
        exit(1);
    }

    //Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    // Determine how many elements each local process should expect
    // and generate array
    if((p-1) != id){
        locMin = floor( (id*n) / p );
        locMax = floor( ( ( (id+1) * (n) ) / p) - 1 );

        //Because the text is split and a match may occur at the end of one text section
        // and continue into another text section, give each section
        // access to the characters of the next section at a length
        // matching the length of the search key (searchLen)
        if (locMax + (searchLen - 1) < n)
            localElems = locMax - locMin + (searchLen - 1);
        else
            localElems = locMax - locMin;
        localText = malloc(localElems * sizeof(localElems));
    }

    //Create array for p-1 and begin distribution of text to each process
    //The array for p-1 will be at least as large as the largest array
    //of all other processes
    if((p-1) == id) {
        int elem_count; //holds the total elements that will be sent to a process
        size_t elements_read; //holds the total elements tat are read from the file

        FILE *iFile = fopen(argv[2], "r");

        //portion out the text
        for(int k = 0; k < p - 1; k++) {
            //calculate the min and max indexes for the process
            min = floor( (k*n) / p );
            max = floor( (((k+1)*n) / p) - 1 );

            //If the string overlap makes the
            if (max + (searchLen - 1) < n)
                elem_count = max - min + (searchLen - 1);
            else
                elem_count = max - min;

            elements_read = fread(localText, 1, elem_count, iFile);

            if(elements_read != elem_count)
                MPI_Abort(MPI_COMM_WORLD, 1);

            MPI_Send(localText, elem_count, MPI_CHAR, k, 1, MPI_COMM_WORLD);

            //fread adjusts the index of the file read, so overlap will throw off the
            // next call to fread. Set the read index of the file back to the max
            // of the previous text section
            fseek(iFile, max, SEEK_SET);
        }

        //find number of elements for process p-1, generate the array and fill
        localElems = ceil(n / p);
        locMax = n;
        locMin = n - localElems;
        localText = malloc(localElems * sizeof(localElems));
        elements_read = fread(localText, 1, localElems, iFile);

        if(elements_read != localElems)
            MPI_Abort(MPI_COMM_WORLD, 1);

        fclose(iFile);
    }
    else {
        //all process other than p-1 will wait to receive data from p-1
        MPI_Recv(localText, localElems, MPI_CHAR, p-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    //Search the text for the search key and store in the local index array
    int *localIndexes = scanText(search, localText, locMin);
    for(int j = 0; j<sizeof(localIndexes)/sizeof(localIndexes[0]); ++j)
        printf("%i \n", localIndexes[j]);
/*
    THE COLLECTION CODE FAILED TO RUN

    //Collect the data from all other processes into p0
    if (0 == id) {

        if(p > 1){
            //generate a holding array of size n (number of bytes in the file) because
            //in the maximal case, each character in the file is a match to the search
            int *buffer = malloc(n * sizeof(int));
            int elem_count;
            printf("here\n");
            //Begin requesting each process to send its data
            for(int k = 1; k < p; ++k){
                min = floor( (k*n) / p );
                max = floor( (((k+1)*n) / p) - 1 );
                elem_count = max-min;

                MPI_Send (&prompt, 1, MPI_INT, k, 1, MPI_COMM_WORLD);

                MPI_Recv (buffer, elem_count, MPI_INT, k, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                printf("%ls \n", buffer);
            }
            free (buffer);
        }
    }
    else {
        MPI_Recv (&prompt, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Send (localIndexes, localElems, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }
*/
    MPI_Finalize();
}
