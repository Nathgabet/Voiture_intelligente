import sys
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation

# Configure the plot
fig = plt.figure(figsize=(8, 8))
ax = fig.add_subplot(111, projection='polar')
ax.set_title("RPLIDAR C1 Real-Time Scan")
ax.set_theta_zero_location('N')  # Set 0 degrees to the Top
ax.set_theta_direction(-1)       # Clockwise rotation [cite: 418, 433]

# Data storage
angles = []
distances = []

# Scatters for the plot
scatter = ax.scatter([], [], s=5, c='red')

def update(frame):
    global angles, distances
    
    # Read a chunk of lines from stdin
    # We read multiple lines to keep up with the high data rate
    for _ in range(50): 
        line = sys.stdin.readline()
        if not line:
            break
        
        try:
            # Expecting "angle,distance" from the C program
            parts = line.strip().split(',')
            if len(parts) >= 2:
                angle_deg = float(parts[0])
                dist_mm = float(parts[1])
                
                # Convert degrees to radians for polar plot [cite: 407]
                angle_rad = np.deg2rad(angle_deg)
                
                # Only keep data within a single 360-degree rotation 
                # (Simple way to clear old data)
                if angle_deg < 1.0 and len(angles) > 100:
                    angles = []
                    distances = []
                
                angles.append(angle_rad)
                distances.append(dist_mm)
        except ValueError:
            continue

    # Update scatter plot
    if angles:
        scatter.set_offsets(np.c_[angles, distances])
    
    return scatter,

# Create animation
# interval=50 updates the plot every 50ms
ani = FuncAnimation(fig, update, interval=50, blit=True, cache_frame_data=False)

plt.show()