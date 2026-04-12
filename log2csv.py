import re
import csv

def convert(input_filename, output_filename):
    """
    Read a log file, extract Stash size and opnum pairs,
    aggregate opnums by stash size, and output to CSV.
    """

    # Dict structure: { stash_size_int: [opnum_int, ...] }
    data_map = {}

    # Regex pattern: "Stash size = <int> ... opnum = <int>"
    pattern = re.compile(r'Stash size\s*=\s*(\d+).*?opnum\s*=\s*(\d+)')

    try:
        with open(input_filename, 'r', encoding='utf-8') as f_in:
            for line in f_in:
                match = pattern.search(line)
                if match:
                    size = int(match.group(1))
                    opnum = int(match.group(2))

                    if size not in data_map:
                        data_map[size] = []

                    data_map[size].append(opnum)

        with open(output_filename, 'w', newline='', encoding='utf-8') as f_out:
            writer = csv.writer(f_out)

            sorted_sizes = sorted(data_map.keys())

            for size in sorted_sizes:
                row = [size] + data_map[size]
                writer.writerow(row)

        print(f"Conversion complete. Output saved to: {output_filename}")

    except FileNotFoundError:
        print(f"Error: file not found: '{input_filename}'")
    except Exception as e:
        print(f"Unexpected error: {e}")

# --- Entry point ---
if __name__ == "__main__":
    input_file = 'logs/Z3.log'
    output_file = 'Z3.csv'

    import os
    if not os.path.exists(input_file):
        print(f"File not found: {input_file}")
        exit(1)

    convert(input_file, output_file)
