import csv
import matplotlib
# Use Agg backend for headless server environments
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

def loadCSV(filename):
    """
    Read a CSV file.
    Format: Stash_Size, opnum1, opnum2, ...
    Returns: a dict {size: [opnum_list]}
    """
    data = {}
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)
            for row in reader:
                if not row: continue
                try:
                    size = int(row[0])
                    opnums = [int(x) for x in row[1:]]
                    data[size] = opnums
                except ValueError:
                    continue
        return data
    except FileNotFoundError:
        print(f"Error: file not found: {filename}")
        return {}

def analysisMany(dataset_list, output_img='swapped_axes_result.png'):
    """
    Analyze and compare multiple datasets (axes swapped).
    X-axis: Opnum (Log Scale)
    Y-axis: Stash Size
    """

    plt.figure(figsize=(10, 8))

    plt.xscale('log')

    for entry in dataset_list:
        data = entry[0]
        name = entry[1]
        color = entry[2]

        if not data:
            continue

        mean_size = []
        mean_opnum = []

        fit_size_raw = []
        fit_opnum_raw = []

        sorted_sizes = sorted(data.keys())

        for size in sorted_sizes:
            opnums = data[size]
            for op in opnums:
                fit_size_raw.append(size)
                fit_opnum_raw.append(op)

            if opnums:
                avg = np.mean(opnums)
                mean_size.append(size)
                mean_opnum.append(avg)

        # Plot average points (swapped axes: X=Opnum, Y=Size)
        plt.scatter(mean_opnum, mean_size, color=color, marker='o', s=60, label=f"{name} (Avg)")

        # Plot fitted line (math model: log(Opnum) = a * Size + b)
        if len(fit_size_raw) > 1:
            try:
                log_opnum = np.log10(fit_opnum_raw)
                slope, intercept = np.polyfit(fit_size_raw, log_opnum, 1)

                line_size = np.linspace(min(fit_size_raw), max(fit_size_raw), 100)

                line_opnum_log = slope * line_size + intercept
                line_opnum = 10 ** line_opnum_log

                # Swapped axes: X=Opnum, Y=Size
                plt.plot(line_opnum, line_size, color=color, linestyle='--', linewidth=2, label=f"{name} (Fit)")
            except Exception as e:
                print(f"Fitting failed: {e}")

    plt.title('Comparison: Opnum vs Stash Size (Axes Swapped)')
    plt.xlabel('Opnum (Log Scale)')
    plt.ylabel('Stash Size')
    plt.grid(True, which="both", ls="-", alpha=0.2)
    plt.legend()
    plt.tight_layout()

    plt.savefig(output_img, dpi=300)
    print(f"Chart saved to: {output_img}")
    plt.close()

# --- Usage example ---
if __name__ == "__main__":
    file1 = 'output.csv'
    file2 = 'result/Z6.csv'
    file3 = 'result/Z5.csv'
    file4 = 'result/Z4.csv'
    file5 = 'result/Z3.csv'

    data1 = loadCSV(file1)
    data2 = loadCSV(file2)
    data3 = loadCSV(file3)
    data4 = loadCSV(file4)
    data5 = loadCSV(file5)

    # Format: [data, name, color]
    datasets_to_plot = [
        [data1, "Experiment A", "blue"],
        [data2, "Experiment Z6", "#FF0000"],
        [data3, "Experiment Z5", "#AA0000"],
        [data4, "Experiment Z4", "#550000"],
        [data5, "Experiment Z3", "#000000"],
    ]

    analysisMany(datasets_to_plot, "multi_analysis.png")
