#include "sort/serial_sort.h"
#include <vector>

using namespace std;

void serialMergeArrays(vector<int>& arr, vector<int>& temp, long left, long mid, long right) {
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

    for (long idx = left; idx <= right; idx++) {
        arr[idx] = temp[idx];
    }
}

void serialMergeSortRec(vector<int>& arr, vector<int>& temp, long left, long right) {
    if (left >= right) return;

    long mid = left + (right - left) / 2;

    serialMergeSortRec(arr, temp, left, mid);
    serialMergeSortRec(arr, temp, mid + 1, right);

    serialMergeArrays(arr, temp, left, mid, right);
}

void serialSort(vector<int>& arr) {
    long arrSize = arr.size();
    if (arrSize < 2) return;
    
    vector<int> temp(arrSize);
    serialMergeSortRec(arr, temp, 0, arrSize - 1);
}