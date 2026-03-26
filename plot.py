import pandas as pd
import matplotlib.pyplot as plt
import os

if not os.path.exists("results/results.csv"):
    print("Error: results/results.csv not found!")
    exit(1)

df = pd.read_csv("results/results.csv")
df = df.dropna(subset=['AvgTime(ms)'])

plt.style.use('ggplot')
DISTRIBUTIONS = ["uniform", "nearly_sorted", "gaussian", "reversed"]

fig_a, axes_a = plt.subplots(2, 2, figsize=(14, 10))
fig_a.suptitle("Execution Time vs. Array Size", fontsize=16)

fig_b, axes_b = plt.subplots(2, 2, figsize=(14, 10))
fig_b.suptitle("OpenMP Speedup vs Threads", fontsize=16)

fig_c, axes_c = plt.subplots(2, 2, figsize=(14, 10))
fig_c.suptitle("CUDA Time vs Block Size", fontsize=16)

fig_d, axes_d = plt.subplots(2, 2, figsize=(14, 10))
fig_d.suptitle("OpenMP Time vs Cutoff Size (Threads=8)", fontsize=16)

axes_list_a = axes_a.flatten()
axes_list_b = axes_b.flatten()
axes_list_c = axes_c.flatten()
axes_list_d = axes_d.flatten()

for idx, DIST in enumerate(DISTRIBUTIONS):
    dist_df = df[df['Distribution'] == DIST]
    if dist_df.empty:
        continue
    TARGET_SIZE = dist_df['Size'].max()

    ax_a = axes_list_a[idx]
    for impl in ['serial', 'omp', 'cuda']:
        impl_df = dist_df[dist_df['Implementation'] == impl].groupby('Size')['AvgTime(ms)'].min().reset_index()
        if not impl_df.empty:
            ax_a.plot(impl_df['Size'], impl_df['AvgTime(ms)'], marker='o', label=impl.capitalize(), linewidth=2)
            
    ax_a.set_title(f"{DIST}")
    ax_a.set_xlabel("Array Size")
    ax_a.set_ylabel("Time (ms)")
    if idx == 0:
        ax_a.legend()

    target_df = dist_df[dist_df['Size'] == TARGET_SIZE]

    ax_b = axes_list_b[idx]
    serial_row = target_df[target_df['Implementation'] == 'serial']
    omp_data = target_df[target_df['Implementation'] == 'omp'].copy()
    omp_data['BlockSizeNum'] = pd.to_numeric(omp_data['BlockSize'], errors='coerce')
    omp_df = omp_data[omp_data['BlockSizeNum'].isna() | (omp_data['BlockSizeNum'] == 10000.0)].copy()
    omp_df = omp_df.drop_duplicates(subset=['Threads'])

    if not serial_row.empty and not omp_df.empty:
        serial_time = serial_row['AvgTime(ms)'].values[0]
        omp_df['Speedup'] = serial_time / omp_df['AvgTime(ms)']
        
        ax_b.plot(omp_df['Threads'], omp_df['Speedup'], marker='s', color='#1f77b4', linewidth=2)
        ax_b.set_title(f"{DIST} (Size={TARGET_SIZE})")
        ax_b.set_xlabel("Threads")
        ax_b.set_ylabel("Speedup Multiplier (x)")

    ax_c = axes_list_c[idx]
    cuda_df = target_df[target_df['Implementation'] == 'cuda']
    if not cuda_df.empty:
        bars = ax_c.bar(cuda_df['BlockSize'].astype(str), cuda_df['AvgTime(ms)'], color='orange', alpha=0.8)
        
        for bar in bars:
            yval = bar.get_height()
            ax_c.text(bar.get_x() + bar.get_width()/2, yval, f'{yval:.1f}', ha='center', va='bottom', fontsize=9)

        ax_c.set_title(f"{DIST} (Size={TARGET_SIZE})")
        ax_c.set_xlabel("Block Size")
        ax_c.set_ylabel("Time (ms)")

    if DIST == "uniform":
        ax_d = axes_list_d[0] 
        cutoff_df = target_df[(target_df['Implementation'] == 'omp') & (target_df['Threads'] == 8)].copy()
        if not cutoff_df.empty:
            cutoff_df['BlockSizeNum'] = pd.to_numeric(cutoff_df['BlockSize'], errors='coerce')
            cutoff_df = cutoff_df.dropna(subset=['BlockSizeNum']).sort_values(by='BlockSizeNum')
            cutoff_df = cutoff_df.drop_duplicates(subset=['BlockSizeNum'])
            if len(cutoff_df) > 1:
                ax_d.plot(cutoff_df['BlockSizeNum'].astype(int).astype(str), cutoff_df['AvgTime(ms)'], marker='^', color='green', linewidth=2)
                ax_d.set_title(f"{DIST} (Threads=8, Size={TARGET_SIZE})")
                ax_d.set_xlabel("Cutoff Size")
                ax_d.set_ylabel("Time (ms)")
        
        for i in range(1, 4):
            fig_d.delaxes(axes_list_d[i])

fig_a.tight_layout()
fig_a.savefig("results/plot_a_execution_time_vs_array_size_combined.png")

fig_b.tight_layout()
fig_b.savefig("results/plot_b_openmp_speedup_vs_threads_combined.png")

fig_c.tight_layout()
fig_c.savefig("results/plot_c_cuda_execution_time_vs_block_size_combined.png")

fig_d.tight_layout()
fig_d.savefig("results/plot_d_openmp_cutoff_tuning_combined.png")

plt.close('all')

