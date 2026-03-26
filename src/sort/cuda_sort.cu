#include "sort/cuda_sort.h"
#include <iostream>
#include <cuda_runtime.h>
#include <climits>

using namespace std;

__global__ void bitonicStep(int* devArr, int j, int k, long paddedSize) {
    long i = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (i >= paddedSize) return;

    long ixj = i ^ j;

    if (ixj > i) {
        if ((i & k) == 0) {
            if (devArr[i] > devArr[ixj]) {
                int temp = devArr[i];
                devArr[i] = devArr[ixj];
                devArr[ixj] = temp;
            }
        } else {
            if (devArr[i] < devArr[ixj]) {
                int temp = devArr[i];
                devArr[i] = devArr[ixj];
                devArr[ixj] = temp;
            }
        }
    }
}

CudaSortTimes cudaSort(vector<int>& arr, int blockSize) {
    CudaSortTimes times;
    long originalSize = arr.size();
    if (originalSize < 2) return times;

    long paddedSize = 1;
    while (paddedSize < originalSize) {
        paddedSize <<= 1;
    }

    if (paddedSize > originalSize) {
        arr.resize(paddedSize, INT_MAX);
    }

    cudaEvent_t startH2D, stopH2D, startComp, stopComp, startD2H, stopD2H;
    cudaEventCreate(&startH2D); cudaEventCreate(&stopH2D);
    cudaEventCreate(&startComp); cudaEventCreate(&stopComp);
    cudaEventCreate(&startD2H); cudaEventCreate(&stopD2H);

    int* devArr;
    long bytes = paddedSize * sizeof(int);
    cudaMalloc((void**)&devArr, bytes);
    
    cudaEventRecord(startH2D);
    cudaMemcpy(devArr, arr.data(), bytes, cudaMemcpyHostToDevice);
    cudaEventRecord(stopH2D);

    long numBlocks = (paddedSize + blockSize - 1) / blockSize;

    cudaEventRecord(startComp);
    for (long k = 2; k <= paddedSize; k <<= 1) {
        for (long j = k >> 1; j > 0; j >>= 1) {
            bitonicStep<<<numBlocks, blockSize>>>(devArr, j, k, paddedSize);
            cudaDeviceSynchronize();
        }
    }
    cudaEventRecord(stopComp);

    cudaEventRecord(startD2H);
    cudaMemcpy(arr.data(), devArr, bytes, cudaMemcpyDeviceToHost);
    cudaEventRecord(stopD2H);

    cudaEventSynchronize(stopD2H);

    float h2d, comp, d2h;
    cudaEventElapsedTime(&h2d, startH2D, stopH2D);
    cudaEventElapsedTime(&comp, startComp, stopComp);
    cudaEventElapsedTime(&d2h, startD2H, stopD2H);

    times.h2d_ms = h2d;
    times.compute_ms = comp;
    times.d2h_ms = d2h;

    cudaEventDestroy(startH2D); cudaEventDestroy(stopH2D);
    cudaEventDestroy(startComp); cudaEventDestroy(stopComp);
    cudaEventDestroy(startD2H); cudaEventDestroy(stopD2H);

    cudaFree(devArr);
    if (paddedSize > originalSize) {
        arr.resize(originalSize);
    }
    return times;
}
