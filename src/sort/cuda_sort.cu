#include "sort/cuda_sort.h"
#include <iostream>
#include <cuda_runtime.h>
#include <climits>
#include <stdexcept>
#include <string>
#include <limits>

using namespace std;

#define CHECK_CUDA(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            throw std::runtime_error(std::string("CUDA Error: ") + cudaGetErrorString(err)); \
        } \
    } while (0)

__global__ void bitonicStep(int* devArr, long j, long k, long paddedSize) {
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
    if (blockSize <= 0) {
        throw runtime_error("Invalid CUDA block size");
    }

    long paddedSize = 1;
    while (paddedSize < originalSize) {
        if (paddedSize > (LONG_MAX >> 1)) {
            throw runtime_error("Array too large for power-of-two padding");
        }
        paddedSize <<= 1;
    }

    if (paddedSize < originalSize) {
        throw runtime_error("Padding overflow detected");
    }

    if (paddedSize > originalSize) {
        arr.resize(paddedSize, INT_MAX);
    }
    
    cudaEvent_t startH2D = nullptr, stopH2D = nullptr, startComp = nullptr, stopComp = nullptr, startD2H = nullptr, stopD2H = nullptr;
    int* devArr = nullptr;
    
    try {
        CHECK_CUDA(cudaEventCreate(&startH2D)); 
        CHECK_CUDA(cudaEventCreate(&stopH2D));
        CHECK_CUDA(cudaEventCreate(&startComp)); 
        CHECK_CUDA(cudaEventCreate(&stopComp));
        CHECK_CUDA(cudaEventCreate(&startD2H)); 
        CHECK_CUDA(cudaEventCreate(&stopD2H));

        if (paddedSize < 0) {
            throw runtime_error("Negative padded size (overflow)");
        }
        if (static_cast<unsigned long>(paddedSize) > (std::numeric_limits<size_t>::max() / sizeof(int))) {
            throw runtime_error("Array too large for device allocation size_t");
        }
        size_t bytes = static_cast<size_t>(paddedSize) * sizeof(int);
        CHECK_CUDA(cudaMalloc((void**)&devArr, bytes));
        
        CHECK_CUDA(cudaEventRecord(startH2D));
        CHECK_CUDA(cudaMemcpy(devArr, arr.data(), bytes, cudaMemcpyHostToDevice));
        CHECK_CUDA(cudaEventRecord(stopH2D));

        long numBlocksLong = (paddedSize + blockSize - 1) / blockSize;
        if (numBlocksLong > INT_MAX) {
            throw runtime_error("Grid size exceeds int limit");
        }
        int numBlocks = static_cast<int>(numBlocksLong);

        CHECK_CUDA(cudaEventRecord(startComp));
        for (long k = 2; k <= paddedSize; k <<= 1) {
            for (long j = k >> 1; j > 0; j >>= 1) {
                bitonicStep<<<numBlocks, blockSize>>>(devArr, j, k, paddedSize);
                CHECK_CUDA(cudaGetLastError());
                CHECK_CUDA(cudaDeviceSynchronize());
            }
        }
        CHECK_CUDA(cudaEventRecord(stopComp));

        CHECK_CUDA(cudaEventRecord(startD2H));
        CHECK_CUDA(cudaMemcpy(arr.data(), devArr, bytes, cudaMemcpyDeviceToHost));
        CHECK_CUDA(cudaEventRecord(stopD2H));

        CHECK_CUDA(cudaEventSynchronize(stopD2H));

        float h2d, comp, d2h;
        CHECK_CUDA(cudaEventElapsedTime(&h2d, startH2D, stopH2D));
        CHECK_CUDA(cudaEventElapsedTime(&comp, startComp, stopComp));
        CHECK_CUDA(cudaEventElapsedTime(&d2h, startD2H, stopD2H));

        times.h2d_ms = h2d;
        times.compute_ms = comp;
        times.d2h_ms = d2h;
    } catch (...) {
        if (startH2D) cudaEventDestroy(startH2D);
        if (stopH2D) cudaEventDestroy(stopH2D);
        if (startComp) cudaEventDestroy(startComp);
        if (stopComp) cudaEventDestroy(stopComp);
        if (startD2H) cudaEventDestroy(startD2H);
        if (stopD2H) cudaEventDestroy(stopD2H);
        if (devArr) cudaFree(devArr);
        if (paddedSize > originalSize && static_cast<long>(arr.size()) != originalSize) arr.resize(originalSize);
        throw;
    }

    cudaEventDestroy(startH2D); cudaEventDestroy(stopH2D);
    cudaEventDestroy(startComp); cudaEventDestroy(stopComp);
    cudaEventDestroy(startD2H); cudaEventDestroy(stopD2H);

    cudaFree(devArr);
    if (paddedSize > originalSize) {
        arr.resize(originalSize);
    }
    return times;
}
