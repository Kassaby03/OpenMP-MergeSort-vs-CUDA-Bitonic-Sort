#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <exception>
#include <algorithm>
#include "utils.h"
#include "sort/serial_sort.h"
#include "sort/omp_sort.h"
#ifdef HAS_CUDA
#include "sort/cuda_sort.h"
#include <cuda_runtime.h>
#endif

using namespace std;

bool isValidDistribution(const string& distType) {
    return distType == "uniform" ||
           distType == "gaussian" ||
           distType == "nearly_sorted" ||
           distType == "reversed";
}

bool isValidImplementation(const string& impl) {
    return impl == "serial" || impl == "omp" || impl == "cuda";
}

void printHelp() {
    cout << "Usage: ./sort_app [options]" << endl;
    cout << "Options:" << endl;
    cout << "  --size INT           Array length" << endl;
    cout << "  --distribution STR   {uniform, gaussian, nearly_sorted, reversed}" << endl;
    cout << "  --seed INT           Random seed" << endl;
    cout << "  --impl STR           {serial, omp, cuda}" << endl;
    cout << "  --threads INT        Number of OpenMP threads" << endl;
    cout << "  --block-size INT     CUDA block size" << endl;
    cout << "  --repeats INT        Number of runs for averaging" << endl;
    cout << "  --output STR         CSV output file path" << endl;
    cout << "  --cutoff INT         OpenMP serial cutoff limit" << endl;
    cout << "  --help               Show this message" << endl;
}

int main(int argc, char** argv) {
    long arrSize = 10000;
    string distType = "uniform";
    int seed = 42;
    string impl = "serial";
    int numThreads = 1;
    int blockSize = 256;
    int numRepeats = 5;
    long cutoff = 10000;
    string outFile = "results/results.csv";

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--size" && i + 1 < argc) {
            arrSize = atol(argv[++i]);
        } else if (arg == "--distribution" && i + 1 < argc) {
            distType = argv[++i];
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (arg == "--impl" && i + 1 < argc) {
            impl = argv[++i];
        } else if (arg == "--threads" && i + 1 < argc) {
            numThreads = atoi(argv[++i]);
        } else if (arg == "--block-size" && i + 1 < argc) {
            blockSize = atoi(argv[++i]);
        } else if (arg == "--repeats" && i + 1 < argc) {
            numRepeats = atoi(argv[++i]);
        } else if (arg == "--cutoff" && i + 1 < argc) {
            cutoff = atol(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            outFile = argv[++i];
        } else if (arg == "--help") {
            printHelp();
            return 0;
        } else {
            cout << "Unknown argument: " << arg << endl;
            printHelp();
            return 1;
        }
    }

    if (arrSize <= 0) {
        cout << "Error: --size must be > 0" << endl;
        return 1;
    }
    if (numRepeats <= 0) {
        cout << "Error: --repeats must be > 0" << endl;
        return 1;
    }
    if (!isValidDistribution(distType)) {
        cout << "Error: invalid --distribution value: " << distType << endl;
        printHelp();
        return 1;
    }
    if (!isValidImplementation(impl)) {
        cout << "Error: invalid --impl value: " << impl << endl;
        printHelp();
        return 1;
    }
    if (impl == "omp" && numThreads <= 0) {
        cout << "Error: --threads must be > 0 for omp" << endl;
        return 1;
    }
    if (impl == "omp" && cutoff <= 0) {
        cout << "Error: --cutoff must be > 0 for omp" << endl;
        return 1;
    }
    if (impl == "cuda" && blockSize <= 0) {
        cout << "Error: --block-size must be > 0 for cuda" << endl;
        return 1;
    }

    cout << "--- Sorting Experiment ---" << endl;
    
#ifdef HAS_CUDA
    cout << "Build mode: CUDA enabled" << endl;
#else
    cout << "Build mode: CPU-only (CUDA not compiled)" << endl;
#endif
    cout << "Size: " << arrSize << ", Dist: " << distType << ", Seed: " << seed << endl;
    cout << "Impl: " << impl << ", Repeats: " << numRepeats << endl;
    if (impl == "omp") cout << "Threads: " << numThreads << endl;
    if (impl == "cuda") cout << "Block Size: " << blockSize << endl;

#ifdef HAS_CUDA
    if (impl == "cuda") {
        cudaError_t warmErr = cudaFree(0);
        if (warmErr != cudaSuccess) {
            cerr << "CUDA warmup failed: " << cudaGetErrorString(warmErr) << endl;
            return 1;
        }
    }
#endif

    double totalTimeMs = 0.0;
    double totalH2D = 0.0;
    double totalComp = 0.0;
    double totalD2H = 0.0;
    bool isSorted = true;

    for (int run = 0; run < numRepeats; run++) {
        vector<int> myArr = generateArray(arrSize, distType, seed + run);
        vector<int> referenceArr = myArr; 

        double runTimeMs = 0.0;

        try {
            if (impl == "serial") {
                double startTimeMs = getTime();
                serialSort(myArr);
                double endTimeMs = getTime();
                runTimeMs = endTimeMs - startTimeMs;
            } else if (impl == "omp") {
                double startTimeMs = getTime();
                ompSort(myArr, numThreads, cutoff);
                double endTimeMs = getTime();
                runTimeMs = endTimeMs - startTimeMs;
            } else if (impl == "cuda") {
#ifdef HAS_CUDA
                double startTimeMs = getTime();
                CudaSortTimes times = cudaSort(myArr, blockSize);
                double endTimeMs = getTime();
                
                runTimeMs = endTimeMs - startTimeMs; 
                totalH2D += times.h2d_ms;
                totalComp += times.compute_ms;
                totalD2H += times.d2h_ms;
#else
                cout << "CUDA not available in this build (nvcc not found at compile time)." << endl;
                return 1;
#endif
            } else {
                cout << "Unknown implementation: " << impl << endl;
                return 1;
            }
        } catch (const exception& e) {
            cerr << "Exception caught during run " << run + 1 << ": " << e.what() << endl;
            return 1;
        }

        totalTimeMs += runTimeMs;

        if (!checkSorted(referenceArr, myArr)) {
            cerr << "ERROR: Array is NOT sorted on run " << run + 1 << "!" << endl;
            cerr << "Aborting experiments due to logical sorting failure." << endl;
            return 1; 
        }
    }

    double avgTimeMs = totalTimeMs / numRepeats;

    cout << "\n=============================================" << endl;
    cout << "              FINAL SUMMARY                  " << endl;
    cout << "=============================================" << endl;
    if (isSorted) {
        cout << " Verified:        Yes (Successfully Sorted)" << endl;
    } else {
        cout << " Verified:        NO! (Sorting Failed)" << endl;
    }
    cout << " Elements Sorted: " << arrSize << endl;
    cout << " Time (Avg):      " << avgTimeMs << " ms" << endl;
    
    if (impl == "cuda") {
        double avgH2D = totalH2D / numRepeats;
        double avgComp = totalComp / numRepeats;
        double avgD2H = totalD2H / numRepeats;
        double totalEventTime = avgH2D + avgComp + avgD2H;
        double overheadMs = std::max(0.0, avgTimeMs - totalEventTime);
        double compPct = (totalEventTime > 0.0) ? ((avgComp / totalEventTime) * 100.0) : 0.0;
        
        cout << "   [GPU Hardware Timings Breakdown]" << endl;
        cout << "   - H2D Trans:   " << avgH2D << " ms" << endl;
        cout << "   - Compute:     " << avgComp << " ms (" << compPct << "% of GPU time)" << endl;
        cout << "   - D2H Trans:   " << avgD2H << " ms" << endl;
        cout << "   - API Overhead & Padding: " << overheadMs << " ms" << endl;
    }
    cout << "=============================================\n" << endl;

    if (isSorted && outFile != "") {
        ofstream out(outFile, ios_base::app);
        if (out.is_open()) {
            out << arrSize << "," << distType << "," << impl << ",";
            if (impl == "omp") out << numThreads << ",";
            else out << "NA,";
            if (impl == "omp") out << cutoff << ",";
            else out << "NA,";
            if (impl == "cuda") out << blockSize << ",";
            else out << "NA,";
            out << numRepeats << "," << avgTimeMs << "\n";
            out.close();
        } else {
            cerr << "ERROR: Could not open output CSV file: " << outFile << endl;
            return 1;
        }
    }

    return 0;
}
