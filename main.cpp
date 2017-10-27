//
// Created by leslie-fang on 2017/10/27.
//
#include "omp.h"
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
    int thread_nums = omp_get_num_threads();
    int thread_rank = omp_get_thread_num();
    cout<<"In "<<thread_rank<<" out of: "<<thread_nums<<" hhh"<<endl;
    return;
}

