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
 
ax.set_rmax(3000)
dots, =ax.plot([], [], label='lidar', color='red', marker='o',markersize =3, linestyle='None')

if lidar.LidarInit() != 0:
    print("Error Open Lidar")
    exit()

previous_distance = mainlidar.distance
points_to_read =250000
angles_buf = np.zeros(points_to_read)
distances_buf = np.zeros(points_to_read)

def update(frame):
    global angles, distances

    for i in  range(points_to_read) :
        if ((mainlidar.distance + 5) > previous_distance ):
            angles_buf[i] = (mainlidar.angle)
            distances_buf[i]= ( mainlidar.distance)

    dots.set_data(angles_buf, distances_buf)

    return dots,

try:
    # ani = FuncAnimation(fig, update, frames=np.arange(0, 10000), blit=True, interval=30)
    ani = FuncAnimation(fig, update, blit =True, interval=30, cache_frame_data=False)
    plt.show()
    
finally:
    print("Stoping lidar ...")  
    lidar.stop_everything()
