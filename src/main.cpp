#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "utils.h"
#include "sort/serial_sort.h"
#include "sort/omp_sort.h"

using namespace std;

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

    cout << "--- Sorting Experiment ---" << endl;
    cout << "Size: " << arrSize << ", Dist: " << distType << ", Seed: " << seed << endl;
    cout << "Impl: " << impl << ", Repeats: " << numRepeats << endl;
    if (impl == "omp") cout << "Threads: " << numThreads << endl;
    if (impl == "cuda") cout << "Block Size: " << blockSize << endl;

    double totalTime = 0.0;
    bool isSorted = true;

    for (int run = 0; run < numRepeats; run++) {
        vector<int> myArr = generateArray(arrSize, distType, seed + run);

        double startTime = getTime();

        if (impl == "serial") {
            serialSort(myArr);
        } else if (impl == "omp") {
            ompSort(myArr, numThreads, cutoff);
        } else if (impl == "cuda") {
            cout << "CUDA not available in this version." << endl;
            return 1;
        } else {
            cout << "Unknown implementation: " << impl << endl;
            return 1;
        }

        double endTime = getTime();
        double runTime = endTime - startTime;
        totalTime += runTime;

        if (!checkSorted(myArr)) {
            cout << "ERROR: Array is NOT sorted on run " << run + 1 << "!" << endl;
            isSorted = false;
        }
    }

    double avgTime = totalTime / numRepeats;
    cout << "\n=============================================" << endl;
    cout << "              FINAL SUMMARY                  " << endl;
    cout << "=============================================" << endl;
    if (isSorted) {
        cout << " Verified:        Yes (Successfully Sorted)" << endl;
    } else {
        cout << " Verified:        NO! (Sorting Failed)" << endl;
    }
    cout << " Elements Sorted: " << arrSize << endl;
    cout << " Average Time:    " << avgTime << " s" << endl;
    cout << "=============================================\n" << endl;

    if (isSorted && outFile != "") {
        ofstream out(outFile, ios_base::app);
        if (out.is_open()) {
            out << arrSize << "," << distType << "," << impl << ",";
            if (impl == "omp") out << numThreads << ",";
            else out << "NA,";
            if (impl == "cuda") out << blockSize << ",";
            else out << "NA,";
            out << numRepeats << "," << avgTime << "\n";
            out.close();
        }
    }

    return 0;
}
