//
// Created by leslie-fang on 2017/10/27.
//
#ifdef _OPENMP
    #include "omp.h"
#endif

#include <iostream>

using namespace std;
void hello(void);

int main(int argc, char **argv){
    int thread_num = atoi(argv[1]);
    cout<<"The number of threads is: "<<thread_num<<endl;
# pragma omp parallel num_threads(thread_num)
    hello();
    return 1;

}

void hello(void){
    #ifdef _OPENMP
        int thread_nums = omp_get_num_threads();
        int thread_rank = omp_get_thread_num();
    #else
        int thread_nums = 1;
        int thread_rank = 0;
    #endif
    cout<<"In "<<thread_rank<<" out of: "<<thread_nums<<" hhh"<<endl;
    return;
}

