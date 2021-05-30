#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double approx_log(int partitions, int trg){
    double i;
    double sum, x, dx;

    dx = ((double)trg - 1.0) / (double) partitions;
    sum = 0.0;

    for (i = 1; i < trg; i+=dx){
        sum += 1/i;
    }

    return sum*dx;

}

int main(){
    int partitions = 1000000;
    int trg = 10;
    double sum;
    sum = approx_log(partitions, trg);
    printf("estimate is %.16f\n", sum);
}
