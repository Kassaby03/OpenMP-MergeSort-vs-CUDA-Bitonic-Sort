#include "utils.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <sys/time.h>
using namespace std;
vector<int> generateArray(long arrSize, const string& distType, int seed) {
    vector<int> arr;
    try {
        arr.resize(arrSize);
    } catch (const exception& e) {
        cerr << "\n[ERROR] Not enough memory or size limit exceeded for an array of size " << arrSize << "!\n" << endl;
        exit(1);
    }
    mt19937 gen(seed);
    if (distType == "uniform") {
        uniform_int_distribution<long> dist(0, 10000000);
        for (long i = 0; i < arrSize; i++) {
            arr[i] = dist(gen);
        }
    } else if (distType == "gaussian") {
        normal_distribution<double> dist(5000000.0, 1000000.0);
        for (long i = 0; i < arrSize; i++) {
            arr[i] = (int)dist(gen);
        }
    } else if (distType == "nearly_sorted") {
        uniform_int_distribution<long> dist(0, 10000000);
        for (long i = 0; i < arrSize; i++) {
            arr[i] = dist(gen);
        }
        sort(arr.begin(), arr.end());
        uniform_int_distribution<long> idxDist(0, arrSize - 1);
        long numSwaps = arrSize / 100;
        for (long i = 0; i < numSwaps; i++) {
            long idx1 = idxDist(gen);
            long idx2 = idxDist(gen);
            int temp = arr[idx1];
            arr[idx1] = arr[idx2];
            arr[idx2] = temp;
        }
    } else if (distType == "reversed") {
        uniform_int_distribution<long> dist(0, 10000000);
        for (long i = 0; i < arrSize; i++) {
            arr[i] = dist(gen);
        }
        sort(arr.begin(), arr.end(), greater<int>());
    } else {
        cout << "Unknown distribution: " << distType << endl;
        uniform_int_distribution<long> dist(0, 10000000);
        for (long i = 0; i < arrSize; i++) {
            arr[i] = dist(gen);
        }
    }
    return arr;
}
bool checkSorted(const vector<int>& arr) {
    long arrSize = arr.size();
    if (arrSize == 0) return true;
    for (long i = 0; i < arrSize - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return false;
        }
    }
    return true;
}
double getTime() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6);
}
