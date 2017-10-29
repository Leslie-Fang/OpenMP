/**
 * Name:
 * Student id:
 * ITSC email:
 */
/*
 * This is a mpi version of bellman_ford algorithm
 * Compile: mpic++ -std=c++11 -o mpi_bellman_ford mpi_bellman_ford.cpp
 * Run: mpiexec -n <number of processes> ./mpi_bellman_ford <input file>, you will find the output file 'output.txt'
 * */

#include <string>
#include <cassert>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <cstring>

#include "mpi.h"

using std::string;
using std::cout;
using std::endl;

#define INF 1000000

/**
 * utils is a namespace for utility functions
 * including I/O (read input file and print results) and matrix dimension convert(2D->1D) function
 */
namespace utils {
    int N; //number of vertices
    int *mat; // the adjacency matrix

    void abort_with_error_message(string msg) {
        std::cerr << msg << endl;
        abort();
    }

    //translate 2-dimension coordinate to 1-dimension
    int convert_dimension_2D_1D(int x, int y, int n) {
        return x * n + y;
    }

    int read_file(string filename) {
        std::ifstream inputf(filename, std::ifstream::in);
        if (!inputf.good()) {
            abort_with_error_message("ERROR OCCURRED WHILE READING INPUT FILE");
        }
        inputf >> N;
        //input matrix should be smaller than 20MB * 20MB (400MB, we don't have too much memory for multi-processors)
        assert(N < (1024 * 1024 * 20));
        mat = (int *) malloc(N * N * sizeof(int));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++) {
                inputf >> mat[convert_dimension_2D_1D(i, j, N)];
            }
        return 0;
    }

    int print_result(bool has_negative_cycle, int *dist) {
        std::ofstream outputf("output.txt", std::ofstream::out);
        if (!has_negative_cycle) {
            for (int i = 0; i < N; i++) {
                if (dist[i] > INF)
                    dist[i] = INF;
                outputf << dist[i] << '\n';
            }
            outputf.flush();
        } else {
            outputf << "FOUND NEGATIVE CYCLE!" << endl;
        }
        outputf.close();
        return 0;
    }
}//namespace utils

// you may add some helper functions here.

/**
 * Bellman-Ford algorithm. Find the shortest path from vertex 0 to other vertices.
 * @param my_rank the rank of current process
 * @param p number of processes
 * @param comm the MPI communicator
 * @param n input size
 * @param *mat input adjacency matrix
 * @param *dist distance array
 * @param *has_negative_cycle a bool variable to recode if there are negative cycles
*/
void bellman_ford(int my_rank, int p, MPI_Comm comm, int n, int *mat, int *dist, bool *has_negative_cycle) {
    //------your code starts from here------
    int loc_n; // I need a local copy for N
    int loc_start, loc_end;
    int *loc_mat; //local matrix
    int *loc_dist; //local distance

    bool *queue1;
    bool *queue2;
    int *loc_enqueue_counter;

    //step 1: broadcast N
    if (my_rank == 0) {
        loc_n = n;
    }
    MPI_Bcast(&loc_n, 1, MPI_INT, 0, comm);

    //step 2: find local task range
    int ave = loc_n / p;
    loc_start = ave * my_rank;
    loc_end = ave * (my_rank + 1);
    if (my_rank == p - 1) {
        loc_end = loc_n;
    }

    //step 3: allocate local memory
    loc_mat = (int *) malloc(loc_n * loc_n * sizeof(int));
    loc_dist = (int *) malloc(loc_n * sizeof(int));
    queue1 = (bool *) malloc(loc_n * sizeof(bool));
    queue2 = (bool *) malloc(loc_n * sizeof(bool));
    loc_enqueue_counter = (int *) calloc(loc_n, sizeof(int));

    //step 4: broadcast matrix mat
    if (my_rank == 0)
        memcpy(loc_mat, mat, sizeof(int) * loc_n * loc_n);
    MPI_Bcast(loc_mat, loc_n * loc_n, MPI_INT, 0, comm);

    //step 5: bellman-ford algorithm
    for (int i = 0; i < loc_n; i++) {
        loc_dist[i] = INF;
    }
    loc_dist[0] = 0;
    queue1[0] = true;

    bool loc_has_change;
    bool has_change;
    for(;;){
        loc_has_change = false;
        memset(queue2, 0, sizeof(bool) * loc_n);// 0 as false
        for(int u = 0; u < loc_n; u++){
            if(queue1[u]){ // if u is active
                for(int v = loc_start; v < loc_end; v++){
                    int weight = loc_mat[utils::convert_dimension_2D_1D(u, v, loc_n)];
                    if (weight < INF) {
                        if (loc_dist[u] + weight < loc_dist[v]) {
                            loc_dist[v] = loc_dist[u] + weight;
                            queue2[v] = true;
                            loc_has_change = true;
                        }
                    }
                }
            }
        }
        MPI_Allreduce(queue2, queue1, loc_n, MPI_CXX_BOOL, MPI_LOR, comm);
        for(int u = 0 ; u < loc_n; u++){
            if(queue1[u]){
                loc_enqueue_counter[u]++;
                if(loc_enqueue_counter[u] >= loc_n) {
                    *has_negative_cycle= true;
                    goto FREE_MEM;
                }
            }
        }

        MPI_Allreduce(&loc_has_change, &has_change, 1, MPI_CXX_BOOL, MPI_LOR, comm);
        if(!has_change){
            break;
        }

        MPI_Allreduce(MPI_IN_PLACE, loc_dist, loc_n, MPI_INT, MPI_MIN, comm);
    }


    //step 6: retrieve results back
    //Hint: use MPI_Gather(or MPI_Gatherv) function
    if(my_rank == 0)
        memcpy(dist, loc_dist, loc_n * sizeof(int));

    //step 7: remember to free memory
    FREE_MEM:
    free(loc_mat);
    free(loc_dist);
    free(queue1);
    free(queue2);
    free(loc_enqueue_counter);

    //------end of your code------
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        utils::abort_with_error_message("INPUT FILE WAS NOT FOUND!");
    }
    string filename = argv[1];

    int *dist;
    bool has_negative_cycle = false;

    //MPI initialization
    MPI_Init(&argc, &argv);
    MPI_Comm comm;

    int p;//number of processors
    int my_rank;//my global rank
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &p);
    MPI_Comm_rank(comm, &my_rank);

    //only rank 0 process do the I/O
    if (my_rank == 0) {
        assert(utils::read_file(filename) == 0);
        dist = (int *) malloc(sizeof(int) * utils::N);
    }

    //time counter
    double t1, t2;
    MPI_Barrier(comm);
    t1 = MPI_Wtime();

    //bellman-ford algorithm
    bellman_ford(my_rank, p, comm, utils::N, utils::mat, dist, &has_negative_cycle);
    MPI_Barrier(comm);

    //end timer
    t2 = MPI_Wtime();

    if (my_rank == 0) {
        std::cerr.setf(std::ios::fixed);
        std::cerr << std::setprecision(6) << "Time(s): " << (t2 - t1) << endl;
        utils::print_result(has_negative_cycle, dist);
        free(dist);
        free(utils::mat);
    }
    MPI_Finalize();
    return 0;
}