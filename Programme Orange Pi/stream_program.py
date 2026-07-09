import ctypes
import os
import time
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation

class Lidar(ctypes.Structure):
    _fields_ = [
        ("fd", ctypes.c_uint8),
        ("motor_speed", ctypes.c_uint8),
        ("errorname",ctypes.c_char),
        ("errorcode", ctypes.c_uint8),
        ("error_count",ctypes.c_double),
        ("angle",ctypes.c_float),
        ("distance",ctypes.c_float),
        ("quality", ctypes.c_uint8)
    ]

lib_path = os.path.abspath('.bin/lidar.so')
lidar = ctypes.CDLL(lib_path)
mainlidar = Lidar.in_dll(lidar, "global_new_lidar")

# Configure the plot
angles = []
distances = []
fig = plt.figure(figsize=(8, 8))
ax = fig.add_subplot(111, projection='polar')
ax.set_title("RPLIDAR C1 Real-Time Scan")
ax.set_theta_zero_location('E')  # Set 0 degrees to the Top
ax.set_theta_direction(1)       # Clockwise rotation [cite: 418, 433]
 

line1, = ax.plot([], [], label='lidar', color='cyan', linewidth=2)
scatter = ax.scatter(12*np.pi, 3000, c='red', s=2, alpha=1)

def update(frame):
    global angles, distances

    angles.append(mainlidar.angle )
    distances.append( mainlidar.distance)

    max_points = 360
    if len(angles) > max_points:
        angles.pop(0)
        distances.pop(0)
        
    line1.set_data(angles, distances)

    return line1

if lidar.LidarInit() != 0:
    print("Error Open Lidar")
    exit()
 

time_start = time.time()
time_stop = time.time()

while((time_stop - time_start)<2):
        
    ani = FuncAnimation(fig, update, frames=np.arange(0, 10000), blit=True, interval=30)

    plt.show()
       
    time_stop = time.time()
lidar.stop_everything()
