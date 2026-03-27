import pandas as pd
import matplotlib.pyplot as plt
import os

if not os.path.exists("results/results.csv"):
    print("Error: results/results.csv not found!")
    exit(1)

df = pd.read_csv("results/results.csv")

if 'Cutoff' not in df.columns:
    df['Cutoff'] = pd.NA
if 'BlockSize' not in df.columns:
    df['BlockSize'] = pd.NA

df['Size'] = pd.to_numeric(df.get('Size'), errors='coerce')
df['ThreadsNum'] = pd.to_numeric(df.get('Threads'), errors='coerce')
df['BlockSizeNum'] = pd.to_numeric(df.get('BlockSize'), errors='coerce')

df = df.dropna(subset=['AvgTime(ms)'])

plt.style.use('ggplot')
DISTRIBUTIONS = ["uniform", "nearly_sorted", "gaussian", "reversed"]
PLOT_A_OMP_THREADS = 8
PLOT_A_CUDA_BLOCK_SIZE = 256

fig_a, axes_a = plt.subplots(2, 2, figsize=(14, 10))
fig_a.suptitle(
    f"Execution Time vs. Array Size (Fixed: OMP threads={PLOT_A_OMP_THREADS}, CUDA block={PLOT_A_CUDA_BLOCK_SIZE})",
    fontsize=16
)

fig_b, axes_b = plt.subplots(2, 2, figsize=(14, 10))
fig_b.suptitle("OpenMP Speedup vs Threads", fontsize=16)

fig_c, axes_c = plt.subplots(2, 2, figsize=(14, 10))
fig_c.suptitle("CUDA Speedup vs Block Size", fontsize=16)

axes_list_a = axes_a.flatten()
axes_list_b = axes_b.flatten()
axes_list_c = axes_c.flatten()

for idx, DIST in enumerate(DISTRIBUTIONS):
    dist_df = df[df['Distribution'] == DIST]
    if dist_df.empty:
        continue
    dist_df = dist_df.copy()

    ax_a = axes_list_a[idx]
    serial_df = dist_df[dist_df['Implementation'] == 'serial'].groupby('Size', as_index=False)['AvgTime(ms)'].mean()
    if not serial_df.empty:
        ax_a.plot(serial_df['Size'], serial_df['AvgTime(ms)'], marker='o', label='Serial', linewidth=2)

    omp_df_a = dist_df[
        (dist_df['Implementation'] == 'omp') &
        (dist_df['ThreadsNum'] == PLOT_A_OMP_THREADS)
    ].groupby('Size', as_index=False)['AvgTime(ms)'].mean()
    if not omp_df_a.empty:
        ax_a.plot(
            omp_df_a['Size'],
            omp_df_a['AvgTime(ms)'],
            marker='o',
            label=f'OMP ({PLOT_A_OMP_THREADS} threads)',
            linewidth=2
        )

    cuda_df_a = dist_df[
        (dist_df['Implementation'] == 'cuda') &
        (dist_df['BlockSizeNum'] == PLOT_A_CUDA_BLOCK_SIZE)
    ].groupby('Size', as_index=False)['AvgTime(ms)'].mean()
    if not cuda_df_a.empty:
        ax_a.plot(
            cuda_df_a['Size'],
            cuda_df_a['AvgTime(ms)'],
            marker='o',
            label=f'CUDA (block {PLOT_A_CUDA_BLOCK_SIZE})',
            linewidth=2
        )
            
    ax_a.set_title(f"{DIST}")
    ax_a.set_xlabel("Array Size")
    ax_a.set_ylabel("Time (ms)")
    if idx == 0:
        ax_a.legend()

    unique_sizes = sorted(dist_df['Size'].dropna().unique())
    TARGET_SIZES = unique_sizes[-2:] if len(unique_sizes) >= 2 else unique_sizes

    ax_b = axes_list_b[idx]
    ax_c = axes_list_c[idx]
    
    colors = ['#1f77b4', '#d62728'] 
    markers = ['s', '^']

    for s_idx, target_size in enumerate(TARGET_SIZES):
        target_df = dist_df[dist_df['Size'] == target_size]
        serial_row = target_df[target_df['Implementation'] == 'serial']
        omp_df = target_df[target_df['Implementation'] == 'omp'].groupby('ThreadsNum', as_index=False)['AvgTime(ms)'].mean()
        if not serial_row.empty and not omp_df.empty:
            serial_time = serial_row['AvgTime(ms)'].mean()
            omp_df = omp_df[omp_df['AvgTime(ms)'] > 1e-9].copy()
            if serial_time > 1e-9 and not omp_df.empty:
                omp_df['Speedup'] = serial_time / omp_df['AvgTime(ms)']
                ax_b.plot(omp_df['ThreadsNum'], omp_df['Speedup'], marker=markers[s_idx % 2], color=colors[s_idx % 2], linewidth=2, label=f'Size={int(target_size)}')
        cuda_df = target_df[target_df['Implementation'] == 'cuda'].groupby('BlockSizeNum', as_index=False)['AvgTime(ms)'].mean()
        if not serial_row.empty and not cuda_df.empty:
            serial_time = serial_row['AvgTime(ms)'].mean()
            cuda_df = cuda_df[cuda_df['AvgTime(ms)'] > 1e-9].copy()
            if serial_time > 1e-9 and not cuda_df.empty:
                cuda_df['Speedup'] = serial_time / cuda_df['AvgTime(ms)']
                ax_c.plot(cuda_df['BlockSizeNum'].astype('Int64').astype(str), cuda_df['Speedup'], marker=markers[s_idx % 2], color=colors[s_idx % 2], linewidth=2, label=f'Size={int(target_size)}')

    ax_b.set_title(f"{DIST}")
    ax_b.set_xlabel("Threads")
    ax_b.set_ylabel("Speedup Multiplier (x)")
    if len(TARGET_SIZES) > 0:
        ax_b.legend()

    ax_c.set_title(f"{DIST}")
    ax_c.set_xlabel("Block Size")
    ax_c.set_ylabel("Speedup Multiplier (x)")
    if len(TARGET_SIZES) > 0:
        ax_c.legend()

fig_a.tight_layout()
fig_a.savefig("results/plot_a.png")

fig_b.tight_layout()
fig_b.savefig("results/plot_b.png")

fig_c.tight_layout()
fig_c.savefig("results/plot_c.png")

plt.close('all')

