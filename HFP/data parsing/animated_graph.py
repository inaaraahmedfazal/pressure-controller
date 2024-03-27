import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
from celluloid import Camera

# Fixed dataset
fixed_data = [0.41000000000000003, 0.38, 0.4, 0.38, 0.35, 0.35, 0.33999999999999997, 0.33999999999999997, 0.06, 0.32, 0.29000000000000004, 0.27, 0.25, 0.24, 0.23, 0.31999999999999995, 0.30000000000000004, 0.24]
timestamps = [0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 16.40000000000009, 17.40000000000009, 18.40000000000009]
data_index = 0

# Function to generate data from the fixed dataset
def generate_data():
    global data_index
    if data_index < len(fixed_data):
        data_x = timestamps[data_index]
        data_y = fixed_data[data_index]
        data_index += 1
        return data_x, data_y
    else:
        return None, None  # Return None if all data points have been used

# Modify the update function accordingly
def update(frame):
    global x, y
    new_x, new_y = generate_data()
    if new_x is not None:
        x.append(new_x)
        y.append(new_y)
        x_np = np.array(x)
        y_np = np.array(y)
        sc.set_offsets(np.column_stack((x_np, y_np)))
    else:
        ani.event_source.stop()
    return sc,

fig, ax = plt.subplots()
new_x, new_y = generate_data()
x = [new_x]
y = [new_y]

# Create scatter plot
sc = ax.scatter(np.zeros(len(fixed_data)), np.zeros(len(fixed_data)))
sc.set_offsets(np.column_stack((timestamps, fixed_data)))

# Set plot labels and title
plt.xlabel('Time')
plt.ylabel('Value')
plt.title('Animated Time-Series Graph')

# Create animation
ani = animation.FuncAnimation(fig, update, interval=1000, frames=len(fixed_data), blit=True)

# Show plot
plt.show()
