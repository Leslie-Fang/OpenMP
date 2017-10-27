//
// Created by leslie-fang on 2017/10/27.
//
#ifdef _OPENMP
    #include "omp.h"
#endif

#include <iostream>

using namespace std;

double f(double x){
    return 2*x;
}

int main(int argc, char **argv){
    double global_result = 0.0;
    double a = 0;
    double b = 10;

    int thread_num = atoi(argv[1]);
    cout<<"The number of threads is: "<<thread_num<<endl;
# pragma omp parallel for num_threads(thread_num) \
    reduction(+:global_result)
    for (int i=0;i<10;i++)
        global_result += (f(i)+f(i+1))*1/2;
    cout<<"The result is: "<<global_result<<endl;
    return 1;
}