import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation
import random

# 1. Setup the figure with a polar projection
fig, ax = plt.subplots(subplot_kw={'projection': 'polar'})

# Initialize empty lists for both data streams
theta_data = []
r1_data = []
r2_data = []

# 2. Create two distinct line objects
line1, = ax.plot([], [], label='Stream A', color='cyan', linewidth=2)
line2, = ax.plot([], [], label='Stream B', color='magenta', linewidth=2, linestyle='--')

# Configure polar plot appearance
ax.set_ylim(0, 100)  # Limits for the radius (r)
ax.legend(loc='upper right')

# 3. Update function called at each interval
def update(frame):
    # Convert frame index to radians (0 to 2*pi)
    # This keeps theta wrapping smoothly around the circle
    current_theta = (frame % 360) * (np.pi / 180)
    
    # Simulate receiving new random data for both streams
    # Random walk logic used here for smoother, more organic visual streaming
    new_r1 = max(0, min(100, (r1_data[-1] + random.randint(-10, 10)) if r1_data else 50))
    new_r2 = max(0, min(100, (r2_data[-1] + random.randint(-12, 12)) if r2_data else 50))
    
    theta_data.append(current_theta)
    r1_data.append(new_r1)
    r2_data.append(new_r2)
    
    # Keep only the last 90 points so the trailing tails clear out gracefully
    max_points = 360
    if len(theta_data) > max_points:
        theta_data.pop(0)
        r1_data.pop(0)
        r2_data.pop(0)
        
    # Update data for both lines
    line1.set_data(theta_data, r1_data)
    line2.set_data(theta_data, r2_data)
    
    # Return both line elements to be redrawn
    return line1, line2

# 4. Run the animation
# blit=True optimizes rendering by only updating the lines
ani = FuncAnimation(fig, update, frames=np.arange(0, 10000), blit=True, interval=30)

plt.show()