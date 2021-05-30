//
//
// BUILD INSTRUCTIONS - mpicc -Wall -o <object name> <file-name>.c
//                      mpirun -np <# procs> <object name> <seed-selection> <matrix file>


#include "alloc_matrix.c"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

struct Room {
    int s1;
    int s2;
    int room_number;
};

int main(int argc, char* argv[])
{

    if(argc < 2){
        printf("Too few arguments. Exiting.");
        exit(1);
    }

    int p;
    int id;
    int rows;
    int cols;
    FILE *matrix_file;
    void *mat_storage, **matrix;
    void *loc_mat_storage, **loc_matrix;
    int errval;
    int room_count;
    double val;



    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if(0 == id){
        matrix_file = fopen(argv[2], "rb");

        if(!matrix_file) {
            printf("Error opening file. Exiting...");
            exit(1);
        }

        //Collect the row and column size
        fread(&rows, sizeof(int), 1, matrix_file);
        fread(&cols, sizeof(int), 1, matrix_file);

        //the number of rooms will be the number of students div by 2
        room_count = rows / 2;
        //printf("rows:%i   cols:%i    rc:%i \n", rows, cols, room_count);

        //Initialize the matrix
        alloc_matrix(rows, cols, sizeof(double), &mat_storage, &matrix, &errval);
        double temp;
        for(int i = 0; i < rows; ++i){
            for(int j = 0; j < cols; ++j){
                fread(&temp, sizeof(double), 1, matrix_file);
                ((double **)matrix)[i][j] = temp;
                //printf("matrix[%i][%i] = %f \n", i, j, ((double **)matrix)[i][j]);
            }
        }

        //Establish seed based on user input. If no repeatable, set seed
        // to time(NULL). Otherwise keep default random() seed of 1.
        if(dtoi(argv[1]) == 0){
            srandom(time(NULL))
        }



    }


    MPI_Finalize();

}
