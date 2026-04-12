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
                    continue  # skip rows with parse errors
        return data
    except FileNotFoundError:
        print(f"Error: file not found: {filename}")
        return {}

def analysisSingle(data, output_img='analysis_result.png'):
    """
    Analyze data and save chart as an image file.
    output_img: output image filename (e.g., result.png)
    """
    if not data:
        print("No data available for analysis.")
        return

    all_x = []
    all_y = []
    mean_x = []
    mean_y = []

    sorted_sizes = sorted(data.keys())

    for size in sorted_sizes:
        opnums = data[size]
        for op in opnums:
            all_x.append(size)
            all_y.append(op)

        if opnums:
            avg = np.mean(opnums)
            mean_x.append(size)
            mean_y.append(avg)

    plt.figure(figsize=(10, 6))

    # A. Raw data scatter plot
    plt.scatter(all_x, all_y, color='gray', alpha=0.5, s=20, label='Raw Data')

    # B. Average values
    plt.scatter(mean_x, mean_y, color='red', marker='x', s=100, linewidth=2, label='Average')

    # C. Log scale Y-axis
    plt.yscale('log')

    # D. Linear fit on log(y)
    if len(all_x) > 1:
        log_y = np.log10(all_y)
        slope, intercept = np.polyfit(all_x, log_y, 1)

        fit_x = np.linspace(min(all_x), max(all_x), 100)
        fit_y_log = slope * fit_x + intercept
        fit_y = 10 ** fit_y_log

        equation = f'Fit: log(y)={slope:.4f}x + {intercept:.2f}'
        plt.plot(fit_x, fit_y, color='blue', linestyle='--', linewidth=2, label=equation)

    plt.title('Analysis of Stash Size vs Opnum (Log Scale)')
    plt.xlabel('Stash Size')
    plt.ylabel('Opnum (Log Scale)')
    plt.grid(True, which="both", ls="-", alpha=0.2)
    plt.legend()
    plt.tight_layout()

    plt.savefig(output_img, dpi=300)
    print(f"Chart saved to: {output_img}")

    plt.close()

if __name__ == "__main__":
    input_csv = './result/Z3.csv'
    output_png = './result/Z3.png'

    dataset = loadCSV(input_csv)
    analysisSingle(dataset, output_png)
