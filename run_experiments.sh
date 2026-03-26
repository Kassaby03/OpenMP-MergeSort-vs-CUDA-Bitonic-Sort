#!/bin/bash
set -eo pipefail

EXEC="./bin/sort_app"
OUT="results/results.csv"


echo "Logging system info..."
{
    echo "=== OS & CPU ==="
    uname -a
    lscpu | grep -E "Model name|CPU\(s\):" || echo "lscpu unavailable"
    echo -e "\n=== GPU ==="
    nvidia-smi --query-gpu=gpu_name,memory.total,driver_version --format=csv || echo "nvidia-smi unavailable"
    echo -e "\n=== COMPILER ==="
    g++ --version | head -n 1
    nvcc --version | tail -n 1
} > results/system_info.txt

echo "Size,Distribution,Implementation,Threads,BlockSize,Repeats,AvgTime(ms)" > $OUT

REPEATS=5
DISTRIBUTIONS=("uniform" "nearly_sorted" "gaussian" "reversed")
SIZES=(1048576 4194304 16777216 15000000)
echo "Running Serial Baselines..."
for size in "${SIZES[@]}"; do
    for dist in "${DISTRIBUTIONS[@]}"; do
        echo "Running Serial: Size=$size Dist=$dist"
        $EXEC --size $size --impl serial --distribution $dist --repeats $REPEATS --output $OUT
    done
done

echo "Running OpenMP..."
THREADS=(1 2 4 8 16)
for size in "${SIZES[@]}"; do
    for dist in "${DISTRIBUTIONS[@]}"; do
        for t in "${THREADS[@]}"; do
            echo "Running OpenMP: Size=$size Dist=$dist Threads=$t"
            $EXEC --size $size --impl omp --distribution $dist --threads $t --repeats $REPEATS --output $OUT
        done
    done
done

echo "Testing OpenMP Cutoffs..."
CUTOFFS=(100 1000 10000 50000 100000)
for c in "${CUTOFFS[@]}"; do
    echo "Running OpenMP Cutoff=$c"
    $EXEC --size 16777216 --impl omp --distribution uniform --threads 8 --cutoff $c --repeats $REPEATS --output $OUT
done

echo "Running CUDA..."
BLOCK_SIZES=(128 256 512)
for size in "${SIZES[@]}"; do
    for dist in "${DISTRIBUTIONS[@]}"; do
        for b in "${BLOCK_SIZES[@]}"; do
            echo "Running CUDA: Size=$size Dist=$dist BlockSize=$b"
            $EXEC --size $size --impl cuda --distribution $dist --block-size $b --repeats $REPEATS --output $OUT
        done
    done
done

echo "Experiments finished. CSV saved to $OUT"

echo "Generating publication-ready plots..."
python3 plot.py
