#--------------------------------------------------------------lib used--------------------------------------------------------------

import matplotlib.pyplot as plt #lib used to make and save the graph. To installed it just type in a cmd "pip install matplotlib" version used 3.9.1
from matplotlib import cm
from matplotlib.colors import LightSource
import numpy as np
#--------------------------------------------------------------Main--------------------------------------------------------------

def plot_polar_point(angle_deg, distance):
    # 1. Convert angle from degrees to radians
    angle_rad = np.radians(angle_deg)
    
    # 2. Calculate Cartesian coordinates
    x = distance * np.cos(angle_rad)
    y = distance * np.sin(angle_rad)
    
    # 3. Plotting
    fig, ax = plt.subplots(figsize=(6, 6))
    
    # Plot the point
    ax.plot(x, y, 'ro', label=f'Point ({angle_deg}°, {distance})')
    
    # Draw a line from the origin to the point for clarity
    ax.plot([0, x], [0, y], 'k--', alpha=0.5)
    
    # Set fixed limits so the origin (0,0) is visible
    limit = distance + 1
    ax.set_xlim(-limit, limit)
    ax.set_ylim(-limit, limit)
    
    ax.axhline(0, color='black', lw=1) # X-axis
    ax.axvline(0, color='black', lw=1) # Y-axis
    ax.grid(True)
    ax.legend()
    ax.set_aspect('equal') # Keep the circle circular
    
    plt.title("Polar to Cartesian Projection")
    plt.show()

def plot_anotherway_polar(angle_deg, distance):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='polar')

    # In polar projection, plot(theta, r)
    ax.plot(np.radians(angle_deg), distance, 'ro') 
    plt.show()

#--------------------------------------------------------------__name__ guard--------------------------------------------------------------
if __name__ == "__main__":
    None
    # Example usage: 45 degrees at a distance of 5
    plot_polar_point(45, 5)
