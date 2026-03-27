# OpenMP MergeSort vs CUDA Bitonic Sort

## Overview

This project implements three highly-optimized sorting algorithms with a unified benchmarking CLI and an automated Python visualization pipeline:

- **Serial Baseline** using a pure, recursive Serial Merge Sort
- **OpenMP MergeSort** with task-based parallelism and dynamically tunable cutoff metrics.
- **CUDA Bitonic Sort** with a safe power-of-two padding strategy and isolated PCIe memory tracking.

## Project Structure

```text
├── Makefile
├── run_experiments.sh      Automated benchmark suite runner
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
└── results/                CSV data, generated PNG plots, and system info
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

The application captures execution metrics in milliseconds (`AvgTime(ms)`) using a monotonic `std::chrono` timer for CPU runs.

For GPU executions, the internal CUDA implementation explicitly isolates execution boundaries using `cudaEventRecord` and returns three distinct hardware metrics:

1. **Host-to-Device (H2D)** memory transfer time
2. **Kernel Compute** execution time
3. **Device-to-Host (D2H)** memory transfer time

### Available Flags

| Flag               | Description                                | Default             |
|--------------------|--------------------------------------------|---------------------|
| `--size INT`       | Array length                               | 10000               |
| `--distribution`   | uniform, gaussian, nearly_sorted, reversed | uniform             |
| `--seed INT`       | Random seed                                | 42                  |
| `--impl`           | serial, omp, cuda                          | serial              |
| `--threads INT`    | OpenMP CPU thread count                    | 1                   |
| `--block-size INT` | CUDA GPU block size                        | 256                 |
| `--cutoff INT`     | OMP serial fallback task limit             | 10000               |
| `--repeats INT`    | Number of runs for averaging               | 5                   |
| `--output PATH`    | CSV output file path                       | results/results.csv |

## Run the Complete Benchmark Suite

```bash
chmod +x run_experiments.sh
./run_experiments.sh
```

> [!IMPORTANT]  
> The `run_experiments.sh` script acts strictly as a test runner. You **must** compile the executable using `make` before executing the benchmark loop.

> [!NOTE]
> CUDA experiments run only when a short runtime CUDA probe succeeds (`--impl cuda`).
> If CUDA is unavailable (not compiled, missing driver/device, or runtime failure), CPU experiments still run and CUDA rows are intentionally absent from `results/results.csv`.

### Automated Visualizations

The test suite logs outputs to `results/results.csv` and triggers `plot.py` to generate three charts:

1. **`plot_a.png`**: Compares fixed configurations across array sizes (`Serial`, `OMP threads=8`, `CUDA block-size=256`) for fair baseline comparisons.
2. **`plot_b.png`**: Shows OpenMP speedup vs serial across thread counts, over the two largest tested sizes (to compare power-of-two and non-power-of-two behavior).
3. **`plot_c.png`**: Shows CUDA speedup vs serial across block sizes (128, 256, 512), over the two largest tested sizes.

## Clean

```bash
make clean
```
