#include "sort/omp_sort.h"
#include <omp.h>
#include <algorithm>

using namespace std;

void mergeArrays(vector<int>& arr, vector<int>& temp, long left, long mid, long right) {
    long i = left;
    long j = mid + 1;
    long k = left;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid) {
        temp[k++] = arr[i++];
    }

    while (j <= right) {
        temp[k++] = arr[j++];
    }

    copy(temp.begin() + left, temp.begin() + right + 1, arr.begin() + left);
}

void mergeSortOmp(vector<int>& arr, vector<int>& temp, long left, long right, long cutoff) {
    long arrSize = right - left + 1;
    
    if (arrSize <= cutoff) {
        sort(arr.begin() + left, arr.begin() + right + 1);
        return;
    }

    long mid = left + (right - left) / 2;

    #pragma omp task shared(arr, temp) if(arrSize > cutoff)
    mergeSortOmp(arr, temp, left, mid, cutoff);

    #pragma omp task shared(arr, temp) if(arrSize > cutoff)
    mergeSortOmp(arr, temp, mid + 1, right, cutoff);

    #pragma omp taskwait

    mergeArrays(arr, temp, left, mid, right);
}

void ompSort(vector<int>& arr, int numThreads, long cutoff) {
    long arrSize = arr.size();
    if (arrSize < 2) return;
    if (numThreads <= 0) numThreads = 1;
    if (cutoff <= 0) cutoff = 1;

    vector<int> temp(arrSize);
    
    omp_set_dynamic(0); 
    omp_set_num_threads(numThreads);

    #pragma omp parallel
    {
        #pragma omp single
        {
            mergeSortOmp(arr, temp, 0, arrSize - 1, cutoff);
        }
    }
}
