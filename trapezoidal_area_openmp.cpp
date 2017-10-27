//
// Created by leslie-fang on 2017/10/27.
//
#ifdef _OPENMP
#include "omp.h"
#endif

#include <iostream>

using namespace std;

double f(double x);
void cal(double a,double b,int n,double * global_result);

int main(int argc, char **argv){
    double global_result = 0;
    double a = 0;
    double b = 10;

    int thread_num = atoi(argv[1]);
    cout<<"The number of threads is: "<<thread_num<<endl;
# pragma omp parallel num_threads(thread_num)
    cal(a,b,50,&global_result);

    cout<<"The result is: "<<global_result<<endl;
    return 1;
}

double f(double x){
    return 2*x;
}

void cal(double a,double b,int n,double * global_result){
    double step = (a-b)/n;
    #ifdef _OPENMP
        int thread_nums = omp_get_num_threads();
        int thread_rank = omp_get_thread_num();
    #else
        int thread_nums = 1;
        int thread_rank = 0;
    #endif
    int cal_num_in_oneThread = n/thread_nums;

    double local_a = a+cal_num_in_oneThread*step*thread_rank;
    double local_b = a+cal_num_in_oneThread*step*(thread_rank+1);
    double local_result = 0;
    for(int i=0;i<cal_num_in_oneThread;i++){
        double start = local_a+i*step;
        double end = local_a+(i+1)*step;
        double temp_area = (f(start)+f(end))*step/2.0;
        local_result += temp_area;
    }
# pragma omp critical
    *global_result += local_result;
    return;
}
