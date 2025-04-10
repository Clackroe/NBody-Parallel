import csv
import os
import re
import matplotlib.pyplot as plt

# Read data from CSV

file_path = 'benchmark_results.csv'


for i in range(8):
    file_p = f"{i}_{file_path}"


    methods = {}
    x_labels = []
    num_threads = -1;

    with open(file_p, 'r') as csvfile:
        reader = csv.reader(csvfile)
        headers = next(reader)
        x_labels = list(map(int, headers[1:]))  # skip 'Method'

        for row in reader:
            method = row[0]
            if num_threads == -1:
                match = re.search(r'\d+', method)
                if match:
                    num_threads = int(match.group(0))

            timings = list(map(float, row[1:]))
            methods[method] = timings

# Plot
    plt.figure(figsize=(10, 6))
    for method, timings in methods.items():
        plt.plot(x_labels, timings, marker='o', label=method)

# Labels and title
    plt.xlabel("Number of Operations")
    plt.ylabel("Time (seconds)")
    plt.title(f"Performance Comparison of Threading Methods -- Threads ({num_threads})")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    if os.path.exists(f"stats_{num_threads}.png"):
        os.remove(f"stats_{num_threads}.png")

    plt.savefig(f"stats_{num_threads}.png")
# Show the plot
    plt.show()

