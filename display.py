import csv
import os
import matplotlib.pyplot as plt

# Read data from CSV
file_path = 'benchmark_results.csv'
methods = {}
x_labels = []

with open(file_path, 'r') as csvfile:
    reader = csv.reader(csvfile)
    headers = next(reader)
    x_labels = list(map(int, headers[1:]))  # skip 'Method'

    for row in reader:
        method = row[0]
        timings = list(map(float, row[1:]))
        methods[method] = timings

# Plot
plt.figure(figsize=(10, 6))
for method, timings in methods.items():
    plt.plot(x_labels, timings, marker='o', label=method)

# Labels and title
plt.xlabel("Number of Operations")
plt.ylabel("Time (seconds)")
plt.title("Performance Comparison of Threading Methods")
plt.legend()
plt.grid(True)
plt.tight_layout()

os.remove("stats.png")

plt.savefig("stats.png")
# Show the plot
plt.show()

