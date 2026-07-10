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
        ("errorname", ctypes.c_char),
        ("errorcode", ctypes.c_uint8),
        ("error_count", ctypes.c_double),
        ("angle", ctypes.c_float),
        ("distance", ctypes.c_float),
        ("quality", ctypes.c_uint8)
    ]

def get_valid_lidar_mask(angles, distances, radius=150, min_neighbors=4):
    """
    Calculates the Cartesian distances between all points and returns a boolean mask
    of points that meet the neighbor density requirement.
    """
    # Convert inputs to numpy arrays
    angles_np = np.array(angles)
    distances_np = np.array(distances)

    # 1. Convert Polar to Cartesian (X, Y) for accurate spatial distance calculation
    # Note: Assuming 'angles' are in degrees. If your Lidar outputs radians, remove np.radians()
    angles_rad = np.radians(angles_np)
    x = distances_np * np.cos(angles_rad)
    y = distances_np * np.sin(angles_rad)

    # 2. Matrix calculation of squared distances (broadcasting)
    dx = x[:, np.newaxis] - x[np.newaxis, :]
    dy = y[:, np.newaxis] - y[np.newaxis, :]

    distance_squared = dx**2 + dy**2
    radius_squared = radius**2

    # 3. Count neighbors within the radius (subtract 1 to ignore the point itself)
    neighbors_count = np.sum(distance_squared <= radius_squared, axis=1) - 1

    # 4. Return mask of valid points
    return neighbors_count >= min_neighbors

# --- Lidar Initialization ---
lib_path = os.path.abspath('Downloads/lidar.so')
lidar = ctypes.CDLL(lib_path)
mainlidar = Lidar.in_dll(lidar, "global_new_lidar")

# Configure the plot
angles = []
distances = []
fig = plt.figure(figsize=(8, 8))
ax = fig.add_subplot(111, projection='polar')
ax.set_title("RPLIDAR C1 Real-Time Scan (Filtered)")
ax.set_theta_zero_location('E')  # Set 0 degrees to the Top
ax.set_theta_direction(1)        # Clockwise rotation

scatter = ax.scatter([], [], c='red', s=2, alpha=1)

if lidar.LidarInit() != 0:
    print("Error Open Lidar")
    exit()

time_start = time.time()
time_stop = time.time()

previous_distance = mainlidar.distance

# --- Data Collection ---
while ((time_stop - time_start) < 2):
    if ((mainlidar.distance + 5) > previous_distance):
        angles.append(mainlidar.angle)
        distances.append(mainlidar.distance)

    previous_distance = mainlidar.distance
    time_stop = time.time()

lidar.stop_everything()

print(f"Raw measurements: {len(distances)}")

# --- Filtering ---
# Apply the mask. Adjust 'radius' and 'min_neighbors' based on your environment scale.
if len(distances) > 0:
    valid_mask = get_valid_lidar_mask(angles, distances, radius=200, min_neighbors=4)

    angles_filtered = np.array(angles)[valid_mask]
    distances_filtered = np.array(distances)[valid_mask]

    print(f"Points kept after filtering: {len(distances_filtered)}")
    print(f"Noise points removed: {len(distances) - len(distances_filtered)}")

    # --- Plotting ---
    # Matplotlib polar projection expects angles in radians for set_offsets
    angles_plot_rad = np.radians(angles_filtered)
    scatter.set_offsets(np.c_[angles_plot_rad, distances_filtered])

    # Adjust axis limits based on data
    if len(distances_filtered) > 0:
        ax.set_rmax(np.max(distances_filtered) + 200)
else:
    print("No data collected to filter.")

plt.savefig("lidarfig.png")