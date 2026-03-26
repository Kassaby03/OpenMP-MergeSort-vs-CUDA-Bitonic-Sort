import pandas as pd
import matplotlib.pyplot as plt
import os

if not os.path.exists("results/results.csv"):
    print("Error: results/results.csv not found!")
    exit(1)

df = pd.read_csv("results/results.csv")
df = df.dropna(subset=['AvgTime(ms)'])

plt.style.use('ggplot')
DIST = 'uniform'

dist_df = df[df['Distribution'] == DIST]
TARGET_SIZE = dist_df['Size'].max()

plt.figure(figsize=(8, 5))

for impl in ['serial', 'omp', 'cuda']:
    impl_df = dist_df[dist_df['Implementation'] == impl].groupby('Size')['AvgTime(ms)'].min().reset_index()
    if not impl_df.empty:
        plt.plot(impl_df['Size'], impl_df['AvgTime(ms)'], marker='o', label=impl.capitalize(), linewidth=2)
        
plt.title(f"Execution Time vs. Array Size ({DIST})")
plt.xlabel("Array Size")
plt.ylabel("Time (ms)")
plt.legend()
plt.tight_layout()
plt.savefig("results/plot_a_execution_time_vs_array_size.png")
plt.close()


target_df = dist_df[dist_df['Size'] == TARGET_SIZE]

serial_row = target_df[target_df['Implementation'] == 'serial']
omp_data = target_df[target_df['Implementation'] == 'omp'].copy()
omp_data['BlockSizeNum'] = pd.to_numeric(omp_data['BlockSize'], errors='coerce')
omp_df = omp_data[omp_data['BlockSizeNum'].isna() | (omp_data['BlockSizeNum'] == 10000.0)].copy()
omp_df = omp_df.drop_duplicates(subset=['Threads'])

if not serial_row.empty and not omp_df.empty:
    serial_time = serial_row['AvgTime(ms)'].values[0]
    omp_df['Speedup'] = serial_time / omp_df['AvgTime(ms)']
    
    plt.figure(figsize=(8, 5))
    plt.plot(omp_df['Threads'], omp_df['Speedup'], marker='s', color='#1f77b4', linewidth=2)
    plt.title(f"OpenMP Speedup vs Threads (Size={TARGET_SIZE})")
    plt.xlabel("Threads")
    plt.ylabel("Speedup Multiplier (x)")
    plt.tight_layout()
    plt.savefig("results/plot_b_openmp_speedup_vs_threads.png")
    plt.close()

cuda_df = target_df[target_df['Implementation'] == 'cuda']
if not cuda_df.empty:
    plt.figure(figsize=(8, 5))
    bars = plt.bar(cuda_df['BlockSize'].astype(str), cuda_df['AvgTime(ms)'], color='orange', alpha=0.8)
    
    for bar in bars:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2, yval, f'{yval:.1f}', ha='center', va='bottom', fontsize=9)

    plt.title(f"CUDA Time vs Block Size (Size={TARGET_SIZE})")
    plt.xlabel("Block Size")
    plt.ylabel("Time (ms)")
    plt.tight_layout()
    plt.savefig("results/plot_c_cuda_execution_time_vs_block_size.png")
    plt.close()

cutoff_df = target_df[(target_df['Implementation'] == 'omp') & (target_df['Threads'] == 8)].copy()
if not cutoff_df.empty:
    cutoff_df['BlockSizeNum'] = pd.to_numeric(cutoff_df['BlockSize'], errors='coerce')
    cutoff_df = cutoff_df.dropna(subset=['BlockSizeNum']).sort_values(by='BlockSizeNum')
    cutoff_df = cutoff_df.drop_duplicates(subset=['BlockSizeNum'])
    if len(cutoff_df) > 1:
        plt.figure(figsize=(8, 5))
        plt.plot(cutoff_df['BlockSizeNum'].astype(int).astype(str), cutoff_df['AvgTime(ms)'], marker='^', color='green', linewidth=2)
        plt.title(f"OpenMP Time vs Cutoff Size (Threads=8, Size={TARGET_SIZE})")
        plt.xlabel("Cutoff Size")
        plt.ylabel("Time (ms)")
        plt.tight_layout()
        plt.savefig("results/plot_d_openmp_cutoff_tuning.png")
        plt.close()

print("Plots successfully simplified and saved to results/ directory!")
