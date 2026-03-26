#ifndef CUDA_SORT_H
#define CUDA_SORT_H

#include <vector>

using namespace std;

struct CudaSortTimes {
    double h2d_ms = 0.0;
    double compute_ms = 0.0;
    double d2h_ms = 0.0;
};

CudaSortTimes cudaSort(vector<int>& arr, int blockSize);

#endif
