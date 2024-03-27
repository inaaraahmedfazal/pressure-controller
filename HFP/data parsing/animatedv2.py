import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from matplotlib import cm
import numpy as np
import matplotlib.animation as animation
import ffmpeg
 
fixed_data = np.array([0.35,
                       0.35,
                       0.33999999999999997, 
                       0.33999999999999997, 
                       0.06, 
                       0.33, 
                       0.32, 
                       0.31, 
                       0.29000000000000004, 
                       0.27, 
                       0.26, 
                       0.255, 
                       0.25, 
                       0.24, 
                       0.23, 
                       0.24000000000000002, 
                       0.29000000000000004, 
                       0.325, 
                       0.34,
                       0.33, 
                       0.328,
                       0.325, 
                       0.31999999999999995, 
                       0.315,
                       0.312,
                       0.30000000000000004,
                       0.07,
                       0.295, 
                       0.28, 
                       0.272,
                       0.265,
                       0.262,
                       0.248,
                       0.245, 
                       0.232,
                       0.227,
                       0.24000000000000002, 
                       0.29000000000000004, 
                       0.32, 
                       0.34])
timestamps = np.array([0.0, 
                       1.0,
                       2.0, 
                       3.0, 
                       4.0, 
                       5.0, 
                       6.0, 
                       7.0, 
                       8.0, 
                       9.0, 
                       10.0, 
                       11.0, 
                       12.0, 
                       13.0, 
                       14.0, 
                       14.066666666666833, 
                       14.683333333333394, 
                       15.100000000000136, 
                       15.416666666666742, 
                       16.4, 
                       17.4, 
                       18.4,
                       19.4,
                       20.4,
                       21.4,
                       22.4,
                       23.4,
                       24.4,
                       25.4,
                       26.4,
                       27.4,
                       28.4,
                       29.4,
                       30.4,
                       31.4,
                       32.4,
                       33.066666666666833, 
                       34.683333333333394, 
                       35.100000000000136, 
                       36.416666666666742])
i = 0

IDEAL_PRESSURE = 0.34
data1x = np.array([0.0,1.0,2.0,3.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0]) 
data1y = np.array([0.35, 0.35, 0.33999999999999997, 0.33999999999999997, 0.33, 0.32, 0.31, 0.29000000000000004, 0.27, 0.26, 0.255, 0.25, 0.24, 0.23])

z1 = np.polyfit(data1x, data1y, 1)
p1 = np.poly1d(z1)

data2x = np.array([18.4,19.4,20.4,21.4,22.4,24.4,25.4,26.4,27.4,28.4,29.4,30.4,31.4,32.4])
data2y = np.array([0.325,0.31999999999999995,0.315,0.312,0.30000000000000004,0.295,0.28,0.272,0.265,0.262,0.248,0.245,0.232,0.227])

z2 = np.polyfit(data2x, data2y, 1)
p2 = np.poly1d(z2)
def main():
    numframes = len(fixed_data)-14
    print(numframes)

    fig = plt.figure()
    scat = plt.scatter(timestamps[:15], fixed_data[:15], label="Pressure")
    plt.scatter([4.0, 23.4], [0.06, 0.07], c='red', label="Outliers")
    plt.xlabel("Time Since Start (min)")
    plt.ylabel("Quadrant Pressure Readings (PSI)")
    plt.title("Pressure Over Time")
    plt.scatter([14.066666666666833, 14.683333333333394, 15.100000000000136, 15.416666666666742, 33.066666666666833, 34.683333333333394, 35.100000000000136, 36.416666666666742], [0.24000000000000002, 0.29000000000000004, 0.325, 0.34, 0.24000000000000002, 0.29000000000000004, 0.32, 0.34], c='purple', label="Refill")
    plt.xlim(timestamps[0] - 0.1, timestamps[14])

    plt.axhline(y=IDEAL_PRESSURE, color='green', linestyle='-', label='Ideal Pressure')
    plt.axhline(y=IDEAL_PRESSURE - 0.1, color='green', linestyle='--')
    plt.axhline(y=IDEAL_PRESSURE + 0.1, color='green', linestyle='--')
    plt.legend()

    ani = animation.FuncAnimation(fig, update_plot, interval=1000, frames=range(numframes),
                                  fargs=(scat,), repeat=False)
    ani.save('symposium_video.mp4', writer='ffmpeg')
    plt.show()

def update_plot(i, scat):
    if i == 1:
        plt.plot(data1x, p1(data1x), 'b--')
    if i == 21:
        plt.plot(data2x, p2(data2x), 'b--')
    plt.xlim(timestamps[i] - 0.1, timestamps[i+14])
    scat.set_offsets(np.column_stack((timestamps[i:i+15], fixed_data[i:i+15])))
    
    
    return scat,

main()