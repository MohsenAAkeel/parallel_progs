// This program will make a numeric approximation of ln(x)
// using the Riemann method. The user inputs x and the number of
// partitions they would like to divide the area under the curve into
//
// BUILD INSTRUCTIONS:
//  $ mpicc -Wall -o <target filename> <source name> -lm
// RUN INSUTRUCTIONS:
//  $ mpirun -np n <target filename> <value to approx> <# of partitions>

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*Approximation of ln(x) by approximation of the area under the curve y = 1/x
 for values of x greater than 1
 @param: partitions - the number of divisions desired for the area under the curve
       trg - the target value the user wishes to approximate the natural log (ln(trg)) of
       id - the process id
       p - the number of processors being used for the computation
 @pre:
 @post: the function has determined the width of each partition and caluclated
        the area of each of those partitions it should calculate based on the
        number of processors being used
 @return: returns the sum of the areas of each rectangle multiplied by the differential (dx)
*/
double approx_log(int partitions, int trg, int id, int p){
    int i;
    double sum, x, dx;

    dx = ((double)trg - 1.0) / (double) partitions;
    sum = 0.0;

    //Each pass of the loop will assign a partition dependent on the number of
    // processors in use
    for (i = id+1; i <= partitions+1; i += p){
        x = dx * ((double)i - 0.5) + 1;
        sum += 1/x;
    }

    return sum*dx;

}

int main(int argc, char* argv[])
{
    //ensure arguments are submitted
    if(argc < 2){
        printf("Too few arguments. Exiting.");
        exit(1);
    }
    int id; //procedure id
    int p; //num procedures
    int trg; //value whose log is determined
    int partitions; //number of partitions
    double local_total; //the total area found by a local process
    double total; //final sum of areas across all processes
    double error; //used to calculate the difference
                  //between the approximation and math.log()
    double e_time; // used to evaluate the elapsed time of computation

    //Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    //Collect the command line variables and convert to int
    trg = atoi(argv[1]);
    partitions = atoi(argv[2]);

    //Check if the command line variables meet requirements
    if(id == 0){
        if(trg < 1 || partitions < 1){
            printf("Partitions must be greater than or equal to 1\n");
            printf("and log target greater than or equal to 2\n");
            MPI_Finalize();
            exit(1);
        }
    }

    //Begin timer
    MPI_Barrier(MPI_COMM_WORLD);
    e_time = - MPI_Wtime();

    //Calculate the approximation and collect results from all tasks
    local_total = approx_log(partitions, trg, id, p);

    MPI_Reduce(&local_total, &total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    //Stop timer
    MPI_Barrier(MPI_COMM_WORLD);
    e_time += MPI_Wtime();

    //ROOT task will calculate the error and display results
    if(id == 0){
        error = fabs(total - log((double)trg));
        printf("%d \t %.16f \t %.16f \t %f\n", trg, total, error, e_time);
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}
