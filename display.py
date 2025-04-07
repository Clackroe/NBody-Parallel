import pandas as pd
import matplotlib.pyplot as plt


df = pd.read_csv("benchmark_results.csv")  

# Display table
print(df)

# Plotting
for method in df['Method'].unique():
    method_df = df[df['Method'] == method]  # Filter data for each method
    plt.plot(method_df['NumBodies'], method_df['Seconds'], marker='o', label=method)

# Customize the plot
plt.xlabel('Number of Bodies')
plt.ylabel('Time (Seconds)')
plt.title('Performance Comparison by Method')
plt.legend(title="Method")
plt.grid(True)
plt.tight_layout()

# Show the plot
plt.show()

