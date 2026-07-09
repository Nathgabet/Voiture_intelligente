import math

# Configuration Constants
CRITICAL_DISTANCE = 20.0  # cm - Emergency stop threshold
WARNING_DISTANCE = 60.0   # cm - Start steering away threshold

def calculate_steering_decision(ultrasonic_cm, gyro_heading, lidar_data):
    """
    Determines the target gyro heading to avoid obstacles.
    lidar_data: list of distances, where index = angle relative to car front (0 to 359)
    """
    
    # 1. Emergency Check (Ultrasonic + Front Lidar)
    # Check front Lidar cone (e.g., -15 to +15 degrees)
    front_lidar_min = min(lidar_data[-15:] + lidar_data[:15]) 
    
    if ultrasonic_cm < CRITICAL_DISTANCE or front_lidar_min < CRITICAL_DISTANCE:
        print("EMERGENCY: Obstacle too close! Backing up.")
        # Command: Reverse, and tell gyro to look for a 90-degree escape turn
        return "REVERSE", (gyro_heading + 90) % 360

    # 2. Vector Field Calculation (Obstacle Avoidance)
    x_force = 0.0
    y_force = 1.0  # Default desire is to move straight ahead (Y-axis)

    for angle, distance in enumerate(lidar_data):
        # We only care about obstacles within our warning zone
        if distance < WARNING_DISTANCE and distance > 0:
            # Convert polar (angle/distance) to Cartesian coordinates (X/Y)
            # 0 degrees is straight ahead, 90 is right, 270 is left
            angle_rad = math.radians(angle)
            
            # Stronger force for closer objects (1 / distance)
            repulsion_weight = (WARNING_DISTANCE - distance) / WARNING_DISTANCE
            
            # Obstacles push us away, so we subtract their vectors
            x_force -= math.sin(angle_rad) * repulsion_weight
            y_force -= math.cos(angle_rad) * repulsion_weight

    # 3. Translate Forces into a Target Gyro Heading
    # Calculate the angle of the resulting free-space vector
    avoidance_angle_rad = math.atan2(x_force, y_force)
    avoidance_angle_deg = math.degrees(avoidance_angle_rad)

    # Convert relative angle to absolute target gyro heading
    target_gyro_heading = (gyro_heading + avoidance_angle_deg) % 360

    # 4. Determine Action Type
    if abs(avoidance_angle_deg) > 10:
        return "STEER", target_gyro_heading
    else:
        return "DRIVE_STRAIGHT", gyro_heading

# --- Example Usage ---
# current_gyro = 0.0 (Facing dead North)
# Imagine an obstacle 30cm away on the right side (90 degrees)
mock_lidar = [100.0] * 360
mock_lidar[90] = 30.0 

action, target_angle = calculate_steering_decision(
    ultrasonic_cm=80.0, 
    gyro_heading=0.0, 
    lidar_data=mock_lidar
)

print(f"Action: {action}, Target Gyro Heading: {target_angle:.2f}°")
# Output will correctly show a target angle shifting to the Left (e.g., ~350°)