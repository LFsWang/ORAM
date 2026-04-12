import re
import matplotlib.pyplot as plt
import os
import numpy as np  # for linear fitting

def readLogDir(dirpath):
    """
    Read all .log files from the specified directory,
    and return a dict {filename: path} sorted alphabetically.
    """
    log_dict = {}
    if not os.path.exists(dirpath):
        print(f"Error: directory not found: {dirpath}")
        return log_dict

    print(f"Scanning directory: {dirpath} ...")

    # Get all .log files and sort them alphabetically
    files = [f for f in os.listdir(dirpath) if f.endswith(".log")]
    files.sort()

    # Insert in order (Python 3.7+ dicts preserve insertion order)
    for filename in files:
        full_path = os.path.join(dirpath, filename)
        log_dict[filename] = full_path

    return log_dict

def _parse_single_log(file_path, use_final=False):
    """
    Internal helper: read a single log file and return sorted x, y data.

    Args:
        file_path (str): path to the log file
        use_final (bool): if True, only keep data after the last
                          "Stash size distribution for" header
    """
    x_data = []
    y_data = []

    # If use_final, do not collect until we see the header
    collecting = not use_final

    if not os.path.exists(file_path):
        print(f"Error: file not found: {file_path}")
        return None, None

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            for line in f:
                if use_final and "Stash size distribution for" in line:
                    # Start collecting; clear previous data to keep only the last section
                    collecting = True
                    x_data = []
                    y_data = []
                    continue

                if collecting:
                    match = re.search(r'Stash size (\d+):\s+(\d+)\s+times', line)
                    if match:
                        size = int(match.group(1))
                        times = int(match.group(2))
                        x_data.append(size)
                        y_data.append(times)

    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return None, None

    if not x_data:
        msg = f"Warning: no matching data found in {file_path}."
        if use_final:
            msg += " (use_final is enabled; make sure the log contains 'Stash size distribution for')"
        print(msg)
        return None, None

    # Sort by x-axis (Stash size)
    sorted_pairs = sorted(zip(x_data, y_data))
    x_sorted, y_sorted = zip(*sorted_pairs)
    return x_sorted, y_sorted

def make_result(log_files_dict, title, output_png, xmax=None, ymax=None, use_final=False):
    """
    Read multiple log files and generate a comparison chart (png).

    Args:
        log_files_dict (dict): {label: file_path}
        title (str): chart title
        output_png (str): output PNG filename
        xmax (float/int, optional): X-axis maximum
        ymax (float/int, optional): Y-axis maximum
        use_final (bool): only use data after "Stash size distribution for"
    """

    plt.figure(figsize=(10, 6))

    has_valid_data = False

    for label, file_path in log_files_dict.items():
        x_data, y_data = _parse_single_log(file_path, use_final)

        if x_data and y_data:
            plt.plot(x_data, y_data, marker='o', linestyle='-', label=label, alpha=0.8)
            has_valid_data = True
        else:
            print(f"Skipping {label}: no valid data")

    if not has_valid_data:
        print("Error: no valid data available for plotting.")
        plt.close()
        return

    plt.yscale('log')
    plt.title(title, fontsize=16)
    plt.xlabel('Stash Size', fontsize=12)
    plt.ylabel('Times (Log Scale)', fontsize=12)
    plt.grid(True, which="both", ls="--", alpha=0.5)

    if xmax is not None:
        plt.xlim(right=xmax)
        plt.xlim(left=0)

    if ymax is not None:
        plt.ylim(top=ymax)

    plt.legend()
    plt.tight_layout()
    plt.savefig(output_png, dpi=300)
    plt.close()

    print(f"Chart saved to: {output_png}")

def make_result_rate(log_files_dict, title, output_png, xmax=None, ymax=None, use_final=False):
    """
    Read multiple log files and generate a probability distribution chart (png).
    Y-axis: proportion of occurrences for each stash size (Probability).
    """

    plt.figure(figsize=(10, 6))

    has_valid_data = False

    for label, file_path in log_files_dict.items():
        x_data, y_data = _parse_single_log(file_path, use_final)

        if x_data and y_data:
            total_count = sum(y_data)

            if total_count > 0:
                y_rate = [y / total_count for y in y_data]

                plt.plot(x_data, y_rate, marker='o', linestyle='-', label=label, alpha=0.8)
                has_valid_data = True
            else:
                print(f"Skipping {label}: total count is 0")
        else:
            print(f"Skipping {label}: no valid data")

    if not has_valid_data:
        print("Error: no valid data available for plotting.")
        plt.close()
        return

    plt.yscale('log')
    plt.title(title, fontsize=24)
    plt.xlabel('Stash Size', fontsize=16)
    plt.ylabel('Probability (Log Scale)', fontsize=16)
    plt.grid(True, which="both", ls="--", alpha=0.5)

    if xmax is not None:
        plt.xlim(right=xmax)
        plt.xlim(left=0)

    if ymax is not None:
        plt.ylim(top=ymax)

    plt.legend()
    plt.tight_layout()
    plt.savefig(output_png, dpi=300)
    plt.close()

    print(f"Chart saved to: {output_png}")

def report_slope(log_files_dict, output_png=None, max_stash=10, use_final=False):
    """
    Compute and print the slope and R^2 of the probability distribution
    for stash sizes 1 to max_stash.
    Method: linear fit on (x, log10(Probability)).

    Args:
        log_files_dict (dict): {label: file_path}
        output_png (str, optional): if provided, save a bar chart of slopes
        max_stash (int): maximum stash size to include in the fit
        use_final (bool): only use data after "Stash size distribution for"
    """

    print("=" * 60)
    print(f"Slope and R^2 Report (Fit on Log10(Probability), Range: 1~{max_stash})")
    print("=" * 60)

    plot_labels = []
    plot_slopes = []

    for label, file_path in log_files_dict.items():
        x_data, y_data = _parse_single_log(file_path, use_final)

        if not x_data or not y_data:
            print(f"[report_slope] log {label} : no valid data")
            continue

        total_count = sum(y_data)
        if total_count == 0:
            continue

        # Prepare fitting data (only stash sizes 1~max_stash with count > 0)
        xs = []
        log_ys = []

        for x, y in zip(x_data, y_data):
            if 1 <= x <= max_stash and y > 0:
                xs.append(x)
                prob = y / total_count
                log_ys.append(np.log10(prob))

        if len(xs) < 2:
            print(f"[report_slope] log {label} : insufficient data points (<2) in range 1~{max_stash}")
            continue

        # Linear fit (degree 1 polynomial)
        slope, intercept = np.polyfit(xs, log_ys, 1)

        # Compute R^2
        predicted_ys = [slope * x + intercept for x in xs]
        ss_res = sum((y_true - y_pred) ** 2 for y_true, y_pred in zip(log_ys, predicted_ys))
        mean_y = np.mean(log_ys)
        ss_tot = sum((y_true - mean_y) ** 2 for y_true in log_ys)

        if ss_tot == 0:
            r2 = 1.0
        else:
            r2 = 1 - (ss_res / ss_tot)

        print(f"[report_slope] log {label} stash data (1~{max_stash}) slope = {slope:.6f}, r2={r2:.6f}")

        plot_labels.append(label)
        plot_slopes.append(abs(slope))

    # Draw bar chart if output_png is specified
    if output_png and plot_labels:
        plt.figure(figsize=(10, 6))

        bars = plt.bar(plot_labels, plot_slopes, color='skyblue', edgecolor='black', alpha=0.7)

        # Label values above each bar
        for bar in bars:
            height = bar.get_height()
            plt.text(bar.get_x() + bar.get_width()/2., height,
                     f'{height:.2f}',
                     ha='center', va='bottom')

        plt.xlabel('Log Name', fontsize=12)
        plt.ylabel('Slope (Absolute Value)', fontsize=12)
        plt.title(f'Slope Comparison (Abs) - Range: 1~{max_stash}', fontsize=14)

        # Rotate labels to avoid overlap
        plt.xticks(rotation=45, ha='right')

        plt.grid(axis='y', linestyle='--', alpha=0.5)
        plt.tight_layout()
        plt.savefig(output_png, dpi=300)
        plt.close()

        print(f"Slope chart saved to: {output_png}")


def report_rate(log_files_dict, max_stash=10, use_final=False):
    """
    Compute the cumulative probability of stash size > max_stash for each log,
    and print the report.
    """

    print("=" * 60)
    print(f"Cumulative Probability Report (Range: 1~{max_stash})")
    print("=" * 60)

    plot_labels = []
    plot_slopes = []

    for label, file_path in log_files_dict.items():
        x_data, y_data = _parse_single_log(file_path, use_final)

        if not x_data or not y_data:
            print(f"[report_rate] log {label} : no valid data")
            continue

        total_count = sum(y_data)
        sub = 0
        for x, y in zip(x_data, y_data):
            if x <= max_stash:
                sub += y

        if total_count == 0:
            continue
        rate = (total_count - sub) / total_count
        # Output as N x 10^-6
        N = rate * 1e6
        # LaTeX table format
        print(f"{label} & {N:.3f} \\\\")

# ==========================================
# Test section
# ==========================================
def fig1():
    test_dir = "./logs/figZ3-6/"
    logs = readLogDir(test_dir)
    make_result(
        log_files_dict=logs,
        title="Stash Distribution (Counts)",
        output_png= test_dir+"figZ3-6_counts.png",
        xmax=25,
        use_final=True
    )
    make_result_rate(
        log_files_dict=logs,
        title="Stash Distribution for Uniform Z (Probability)",
        output_png= test_dir+"figZ3-6_rate.png",
        xmax=25,
        use_final=True
    )


def fig2():
    test_dir = "./logs/figZ45-Zmid/"
    logs = readLogDir(test_dir)
    make_result(
        log_files_dict=logs,
        title="Stash Distribution (Counts)",
        output_png= test_dir+"figZ45-Zmid_counts.png",
        xmax=25,
        use_final=True
    )
    make_result_rate(
        log_files_dict=logs,
        title="",
        output_png= test_dir+"figZ45-Zmid_rate.png",
        xmax=25,
        use_final=True
    )
def tab3():
    test_dir = "./logs/tableZ5x0-14/"
    logs = readLogDir(test_dir)
    make_result_rate(
        log_files_dict=logs,
        title="Stash Distribution (Probability)",
        output_png= test_dir+"tableZ5x0-14_counts.png",
        xmax=25,
        use_final=True
    )
    report_rate(
        log_files_dict=logs,
        max_stash=10,
        use_final=True
    )

if __name__ == "__main__":
    fig1()
    fig2()
    tab3()

    exit(0)  # pass, left for manual testing

    test_dir = "./logs/Test3/"
    logs_from_dir = readLogDir(test_dir)
    print("Files found:", logs_from_dir)

    # Merging dicts example (Python 3.9+ supports |, or use {**d1, **d2})
    more_logs = {"Z5": "some/path/file.log"}
    combined_logs = {**logs_from_dir, **more_logs}

    make_result(
        log_files_dict=logs_from_dir,
        title="Stash Distribution (Counts)",
        output_png="stash_counts.png",
        xmax=5,
        use_final=True
    )

    make_result_rate(
        log_files_dict=logs_from_dir,
        title="Stash Distribution for Uniform Z (Probability)",
        output_png="stash_rate.png",
        xmax=25,
        use_final=True
    )
