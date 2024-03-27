import re
import matplotlib.pyplot as plt
import numpy as np

# Open the log file
with open('final_trial.txt', 'r') as file:
    log_content = file.read()

# Use regular expressions to find minute averages
minute_averages = re.findall(r'Average for Q(\d+): ([\d.]+)', log_content)

# Prepare data for plotting
data = {'Q1': [], 'Q2': [], 'Q3': [], 'Q4': []}

for q, value in minute_averages:
    data['Q' + q].append(float(value))

# Create scatter plot
fig, ax = plt.subplots(figsize=(10, 6))

for q, values in data.items():
    # Calculate mean and standard deviation
    mean = np.mean(values)
    std_dev = np.std(values)

    # Filter out outliers
    filtered_values = [x for x in values if (x >= mean - 1.5 * std_dev) and (x <= mean + 1.5 * std_dev)]
    
    # Plot filtered data
    ax.scatter(range(1, len(filtered_values) + 1), filtered_values, label=q)

    # Fit a linear trendline to filtered data
    x = np.array(range(1, len(filtered_values) + 1))
    coeffs = np.polyfit(x, filtered_values, 1)
    trendline = np.poly1d(coeffs)
    ax.plot(x, trendline(x), linestyle='--')

ax.set_title('Minute Averages with Linear Trendlines (Outliers Removed)')
ax.set_xlabel('Minute')
ax.set_ylabel('Pressure')
ax.grid(True)
ax.legend()

plt.show()