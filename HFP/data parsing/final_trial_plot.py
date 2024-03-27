import matplotlib.pyplot as plt
from datetime import datetime
import numpy as np

# Provided data
data = """
18:59:34.586 -> MINUTE AVERAGES:
18:59:34.586 ->
18:59:34.586 -> Average for Q1: 0.57
18:59:34.632 ->
18:59:34.632 -> Average for Q2: 0.51
18:59:34.679 ->
18:59:34.679 -> Average for Q3: 0.32
18:59:34.679 ->
18:59:34.679 -> Average for Q4: 0.51
18:59:34.726 ->
18:59:34.726 ->
18:59:34.726 -> user is detected to be seated
19:00:34.610 ->
19:00:34.610 -> MINUTE AVERAGES:
19:00:34.610 ->
19:00:34.610 -> Average for Q1: 0.57
19:00:34.658 ->
19:00:34.658 -> Average for Q2: 0.51
19:00:34.658 ->
19:00:34.658 -> Average for Q3: 0.29
19:00:34.658 ->
19:00:34.658 -> Average for Q4: 0.51
19:00:34.704 ->
19:00:34.704 ->
19:00:34.704 -> user is detected to be seated
19:01:34.620 ->
19:01:34.620 -> MINUTE AVERAGES:
19:01:34.620 ->
19:01:34.620 -> Average for Q1: 0.60
19:01:34.620 ->
19:01:34.620 -> Average for Q2: 0.49
19:01:34.665 ->
19:01:34.665 -> Average for Q3: 0.31
19:01:34.665 ->
19:01:34.665 -> Average for Q4: 0.49
19:01:34.712 ->
19:01:34.712 ->
19:01:34.712 -> user is detected to be seated
19:02:34.583 ->
19:02:34.583 -> MINUTE AVERAGES:
19:02:34.583 ->
19:02:34.583 -> Average for Q1: 0.60
19:02:34.630 ->
19:02:34.630 -> Average for Q2: 0.50
19:02:34.678 ->
19:02:34.678 -> Average for Q3: 0.29
19:02:34.678 ->
19:02:34.678 -> Average for Q4: 0.50
19:02:34.678 ->
19:02:34.678 ->
19:02:34.678 -> user is detected to be seated
19:03:34.608 ->
19:03:34.608 -> MINUTE AVERAGES:
19:03:34.608 ->
19:03:34.608 -> Average for Q1: 0.59
19:03:34.608 ->
19:03:34.608 -> Average for Q2: 0.51
19:03:34.655 ->
19:03:34.655 -> Average for Q3: 0.26
19:03:34.655 ->
19:03:34.655 -> Average for Q4: 0.51
19:03:34.701 ->
19:03:34.701 ->
19:03:34.701 -> user is detected to be seated
19:04:34.574 ->
19:04:34.574 -> MINUTE AVERAGES:
19:04:34.574 ->
19:04:34.574 -> Average for Q1: 0.60
19:04:34.619 ->
19:04:34.619 -> Average for Q2: 0.50
19:04:34.666 ->
19:04:34.666 -> Average for Q3: 0.26
19:04:34.666 ->
19:04:34.666 -> Average for Q4: 0.51
19:04:34.713 ->
19:04:34.713 ->
19:04:34.713 -> user is detected to be seated
19:05:34.566 ->
19:05:34.566 -> MINUTE AVERAGES:
19:05:34.566 ->
19:05:34.566 -> Average for Q1: 0.61
19:05:34.613 ->
19:05:34.613 -> Average for Q2: 0.49
19:05:34.659 ->
19:05:34.659 -> Average for Q3: 0.25
19:05:34.659 ->
19:05:34.659 -> Average for Q4: 0.50
19:05:34.707 ->
19:05:34.707 ->
19:05:34.707 -> user is detected to be seated
19:06:34.603 ->
19:06:34.603 -> MINUTE AVERAGES:
19:06:34.603 ->
19:06:34.603 -> Average for Q1: 0.61
19:06:34.603 ->
19:06:34.603 -> Average for Q2: 0.48
19:06:34.650 ->
19:06:34.650 -> Average for Q3: 0.25
19:06:34.650 ->
19:06:34.650 -> Average for Q4: 0.49
19:06:34.697 ->
19:06:34.697 ->
19:06:34.697 -> user is detected to be seated
19:07:34.572 ->
19:07:34.572 -> MINUTE AVERAGES:
19:07:34.572 ->
19:07:34.572 -> Average for Q1: 0.14
19:07:34.623 ->
19:07:34.623 -> Average for Q2: 0.75
19:07:34.664 ->
19:07:34.664 -> Average for Q3: -0.03
19:07:34.664 ->
19:07:34.664 -> Average for Q4: 0.82
19:07:34.664 ->
19:07:34.664 ->
19:07:34.664 -> user is detected to be seated
19:08:34.595 ->
19:08:34.595 -> MINUTE AVERAGES:
19:08:34.595 ->
19:08:34.595 -> Average for Q1: 0.50
19:08:34.595 ->
19:08:34.595 -> Average for Q2: 0.58
19:08:34.640 ->
19:08:34.640 -> Average for Q3: 0.23
19:08:34.640 ->
19:08:34.640 -> Average for Q4: 0.59
19:08:34.687 ->
19:08:34.687 ->
19:08:34.687 -> user is detected to be seated
19:09:34.584 ->
19:09:34.584 -> MINUTE AVERAGES:
19:09:34.584 ->
19:09:34.584 -> Average for Q1: 0.51
19:09:34.630 ->
19:09:34.630 -> Average for Q2: 0.58
19:09:34.630 ->
19:09:34.630 -> Average for Q3: 0.20
19:09:34.630 ->
19:09:34.630 -> Average for Q4: 0.56
19:09:34.677 ->
19:09:34.677 ->
19:09:34.677 -> user is detected to be seated
19:10:34.582 ->
19:10:34.582 -> MINUTE AVERAGES:
19:10:34.582 ->
19:10:34.582 -> Average for Q1: 0.51
19:10:34.582 ->
19:10:34.582 -> Average for Q2: 0.58
19:10:34.628 ->
19:10:34.628 -> Average for Q3: 0.18
19:10:34.628 ->
19:10:34.628 -> Average for Q4: 0.55
19:10:34.676 ->
19:10:34.676 ->
19:10:34.676 -> user is detected to be seated
19:11:34.586 ->
19:11:34.586 -> MINUTE AVERAGES:
19:11:34.586 ->
19:11:34.586 -> Average for Q1: 0.49
19:11:34.586 ->
19:11:34.586 -> Average for Q2: 0.58
19:11:34.631 ->
19:11:34.631 -> Average for Q3: 0.16
19:11:34.631 ->
19:11:34.631 -> Average for Q4: 0.55
19:11:34.679 ->
19:11:34.679 ->
19:11:34.679 -> user is detected to be seated
19:12:34.563 ->
19:12:34.563 -> MINUTE AVERAGES:
19:12:34.563 ->
19:12:34.563 -> Average for Q1: 0.50
19:12:34.609 ->
19:12:34.609 -> Average for Q2: 0.56
19:12:34.609 ->
19:12:34.609 -> Average for Q3: 0.15
19:12:34.609 ->
19:12:34.609 -> Average for Q4: 0.52
19:12:34.656 ->
19:12:34.656 ->
19:12:34.656 -> user is detected to be seated
19:13:34.565 ->
19:13:34.565 -> MINUTE AVERAGES:
19:13:34.565 ->
19:13:34.565 -> Average for Q1: 0.51
19:13:34.614 ->
19:13:34.614 -> Average for Q2: 0.56
19:13:34.614 ->
19:13:34.614 -> Average for Q3: 0.14
19:13:34.614 ->
19:13:34.614 -> Average for Q4: 0.50
19:13:34.660 ->
19:13:34.660 ->
19:13:34.660 -> user is detected to be seated
19:13:34.707 -> checking quad
19:13:34.707 -> set to refill
19:13:35.547 -> action state set to inflate
19:13:35.547 -> refill Mode
19:13:35.594 -> Should enter refill routine
19:13:38.573 -> abs diff:  0.23
19:13:38.573 -> delay time:  19082
19:14:00.653 -> abs diff:  0.12
19:14:00.653 -> delay time:  11974
19:14:15.601 -> abs diff:  0.10
19:14:15.601 -> delay time:  10451
19:14:29.088 -> abs diff:  0.07
19:14:29.088 -> delay time:  8928
19:14:40.989 -> abs diff:  0.05
19:14:40.989 -> delay time:  7405
19:14:51.402 -> abs diff:  0.02
19:14:51.402 -> delay time:  5374
19:14:59.783 -> abs diff:  0.00
19:14:59.783 -> stop refilling
19:15:58.778 ->
19:15:58.778 -> MINUTE AVERAGES:
19:15:58.778 ->
19:15:58.778 -> Average for Q1: 0.50
19:15:58.823 ->
19:15:58.823 -> Average for Q2: 0.57
19:15:58.869 ->
19:15:58.869 -> Average for Q3: 0.29
19:15:58.869 ->
19:15:58.869 -> Average for Q4: 0.44
19:15:58.869 ->
19:15:58.869 ->
19:15:58.869 -> user is detected to be seated
19:16:58.788 ->
19:16:58.788 -> MINUTE AVERAGES:
19:16:58.788 ->
19:16:58.788 -> Average for Q1: 0.52
19:16:58.836 ->
19:16:58.836 -> Average for Q2: 0.56
19:16:58.836 ->
19:16:58.836 -> Average for Q3: 0.27
19:16:58.836 ->
19:16:58.836 -> Average for Q4: 0.42
19:16:58.883 ->
19:16:58.883 ->
19:16:58.883 -> user is detected to be seated

19:17:58.773 -> MINUTE AVERAGES:
19:17:58.773 ->
19:17:58.773 -> Average for Q1: 0.52
19:17:58.820 ->
19:17:58.820 -> Average for Q2: 0.57
19:17:58.820 ->
19:17:58.820 -> Average for Q3: 0.21
19:17:58.820 ->
19:17:58.820 -> Average for Q4: 0.39
"""

IDEAL_PRESSURE = 0.34

# Function to convert timestamp to minutes
def timestamp_to_minutes(timestamp):
    t = datetime.strptime(timestamp, "%H:%M:%S.%f")
    return t.hour * 60 + t.minute + t.second / 60

# Extracting timestamps and minute averages for quadrant 3
timestamps = []
Q3_averages = []



lines = data.strip().split('\n')
ctr = 0
for line in lines:
    if "-> Average for Q3" in line:
        timestamps.append(line.split(" -> ")[0])
        value = float(line.split(": ")[1])
        Q3_averages.append(value)
        # if ctr < 15:
        #     Q3_averages.append(value + 0.09)
        # else:
        #     Q3_averages.append(value + 0.03)
        ctr += 1
print(Q3_averages)
# Convert timestamps to minutes since the start
start_time = timestamp_to_minutes(timestamps[0])
minutes = [timestamp_to_minutes(ts) - start_time for ts in timestamps]
print(minutes)

# Extract the first 15 data points for quadrant 3
first_15_Q3_minutes = minutes[:15]
first_15_Q3_averages = Q3_averages[:15]

# Calculate mean and standard deviation of the first 15 data points
mean_Q3 = np.mean(first_15_Q3_averages)
std_dev_Q3 = np.std(first_15_Q3_averages)

# Filter out data points within 1.5 standard deviations of the mean
filtered_minutes = []
filtered_averages = []
outliers_minutes = []
outliers_averages = []

for i in range(len(first_15_Q3_averages)):
    if abs(first_15_Q3_averages[i] - mean_Q3) <= 1.5 * std_dev_Q3:
        filtered_minutes.append(first_15_Q3_minutes[i])
        filtered_averages.append(first_15_Q3_averages[i])
    else:
        outliers_minutes.append(first_15_Q3_minutes[i])
        outliers_averages.append(first_15_Q3_averages[i])

# Fit a trendline to the filtered data
z = np.polyfit(filtered_minutes, filtered_averages, 1)
p = np.poly1d(z)

refill_timestamps = []
refill_minutes = []
refill_values = []

ctr = 0

for line in lines:
    if "-> abs diff" in line:
        # if ctr % 2 == 0:
        refill_timestamps.append(line.split(" -> ")[0])
        value = IDEAL_PRESSURE - float(line.split(": ")[1])
        refill_values.append(value)
        ctr += 1

# Convert timestamps to minutes since the start
refill_minutes = [timestamp_to_minutes(ts) - start_time for ts in refill_timestamps]

print(refill_values)
print(refill_minutes)

# Plotting
plt.figure(figsize=(10, 4))
plt.scatter(minutes[:-1], Q3_averages[:-1], label='Pressure', color='blue')
plt.plot(filtered_minutes, p(filtered_minutes), 'b--', label='Trendline')
plt.scatter(outliers_minutes, outliers_averages, color='red', label=' Flagged Outliers')
plt.scatter(refill_minutes[1:], refill_values[1:], color='purple', label="Pressure - Refilling")
plt.xlabel('Time Since Start (min)', fontsize=13)
plt.ylabel('Pressure Averaged Per Minute (PSI)', fontsize=13)
plt.title('Real Trial: Pressure in Leaky Quadrant', fontsize=18)
plt.axhline(y=IDEAL_PRESSURE, color='green', linestyle='-', label='Ideal Pressure')
plt.axhline(y=IDEAL_PRESSURE - 0.1, color='green', linestyle='--')
plt.axhline(y=IDEAL_PRESSURE + 0.1, color='green', linestyle='--')
plt.legend(fontsize=12)
plt.grid(False)
plt.show()