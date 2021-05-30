// This program will calculate make a numeric approximation of ln(x)
// using the Riemann method. The user inputs x and the number of
// partitions they would like to divide the area under the curve into
//
// BUILD INSTRUCTIONS: This program uses the math library. Append '-lm' to the build command
// This program also uses MPI, use the MPI wrapper to build ('mpicc')
// $ mpicc -Wall -o <target filename> <source name> -lm

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double approx_log(int partitions, int trg, int id, int p){
    int i;
    double sum, x, dx;

    dx = ((double)trg - 1.0) / (double) partitions;
    sum = 0.0;

    for (i = id+2; i <= partitions+1; i += p){
        x = dx * ((double)i - 0.5) + 1;
        sum += 1/x;
    }

    return sum*dx;

}

int main(int argc, char* argv[])
{
    if(argc < 2){
        printf("Too few arguments. Exiting.");
        exit(1);
    }
    int id; //procedure id
    int p; //num procedures
    int trg; //value whose log is determined
    int partitions; //number of partitions
    double local_total;
    double total; //final sum of areas
    double log_calc;
    double error;
    double e_time;

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
        log_calc = log((double)trg);
        error = fabs(total - log_calc);
        printf("%d \t %.16f \t %.16f \t %f\n", trg, total, error, e_time);
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}
