# Project 2 — OpenMP MergeSort vs CUDA Bitonic Sort

## Overview
This project implements three highly-optimized sorting algorithms with a unified benchmarking CLI and an automated Python visualization pipeline:
- **Serial Baseline** using a pure, recursive Serial Merge Sort
- **OpenMP MergeSort** with task-based parallelism and dynamically tunable cutoff metrics.
- **CUDA Bitonic Sort** with a safe power-of-two padding strategy and isolated PCIe memory tracking.

## Project Structure
```text
├── Makefile
├── run_experiments.sh      Automated suite & markdown report generator
├── plot.py                 Dynamically parses CSV arrays to build PNG graphs
├── include/
│   ├── utils.h             Array generation and timing utilities
│   └── sort/
│       ├── serial_sort.h
│       ├── omp_sort.h
│       └── cuda_sort.h
├── src/
│   ├── main.cpp            CLI entry point & GPU timer aggregations
│   ├── utils.cpp           Array generation and validation logic
│   └── sort/
│       ├── serial_sort.cpp
│       ├── omp_sort.cpp
│       └── cuda_sort.cu
├── bin/                    Compiled executable (generated)
├── obj/                    Object files (generated)
└── results/                CSV data, generated PNG plots, and REPORT.md
```

## Build
```bash
make
```

## Usage
```bash
./bin/sort_app --size 16777216 --impl omp --threads 8 --distribution uniform --repeats 5
```

### Precision Timing & Benchmarking
The application captures ultra-precise, high-resolution execution metrics natively in milliseconds (`AvgTime(ms)`) using `std::chrono::high_resolution_clock`. 

For GPU executions, the internal CUDA implementation explicitly isolates execution boundaries using `cudaEventRecord`, outputting three distinct hardware metrics directly to the terminal: 
1. **Host-to-Device (H2D)** memory transfer time
2. **Kernel Compute** execution time
3. **Device-to-Host (D2H)** memory transfer time

### Available Flags
| Flag             | Description                          | Default          |
|------------------|--------------------------------------|------------------|
| `--size INT`     | Array length                         | 10000            |
| `--distribution` | uniform, gaussian, nearly_sorted, reversed | uniform    |
| `--seed INT`     | Random seed                          | 42               |
| `--impl`         | serial, omp, cuda                    | serial           |
| `--threads INT`  | OpenMP CPU thread count              | 1                |
| `--block-size INT`| CUDA GPU block size                 | 256              |
| `--cutoff INT`   | OMP serial fallback task limit       | 10000            |
| `--repeats INT`  | Number of runs for averaging         | 5                |
| `--output PATH`  | CSV output file path                 | results.csv      |

## Run the Complete Benchmark Suite
```bash
chmod +x run_experiments.sh
./run_experiments.sh
```

> [!IMPORTANT]  
> The `run_experiments.sh` script acts strictly as a test runner. You **must** compile the executable using `make` before executing the benchmark loop.

### Automated Visualizations
The test suite securely logs outputs to `results/results.csv` and triggers the `plot.py` pipeline. This creates an automated `results/REPORT.md` file containing four publication-ready charts:

1. **`plot_a_execution_time_vs_array_size.png`**: Evaluates multi-algorithm scaling boundaries across all active array sizes.
2. **`plot_b_openmp_speedup_vs_threads.png`**: Maps multi-core task saturation using standard speedup multipliers.
3. **`plot_c_cuda_execution_time_vs_block_size.png`**: Groups executions to highlight optimal GPU thread hierarchy tuning.
4. **`plot_d_openmp_cutoff_tuning.png`**: Visualizes the exact tipping point of OpenMP parallel task-creation overhead via targeted cutoff thresholds.

## Clean
```bash
make clean
```
