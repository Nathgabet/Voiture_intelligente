import torch
import torch.nn as nn
import torch.nn.functional as F

class RCCarCNN(nn.Module):
    def __init__(self):
        super(RCCarCNN, self).__init__()
        
        # Input shape: (Batch_Size, 1 Channel, 8 Features)
        
        # Layer 1: 1D Convolution
        # Inputs: 1 channel -> Outputs: 16 channels. Kernel size 3 reduces 8 features to 6.
        self.conv1 = nn.Conv1d(in_channels=1, out_channels=16, kernel_size=3, stride=1, padding=0)
        
        # Layer 2: 1D Convolution
        # Inputs: 16 channels -> Outputs: 32 channels. Kernel size 3 reduces 6 features to 4.
        self.conv2 = nn.Conv1d(in_channels=16, out_channels=32, kernel_size=3, stride=1, padding=0)
        
        # Flattened size calculation: 32 channels * 4 remaining features = 128
        
        # Layer 3: Fully Connected (Dense) Layer
        self.fc1 = nn.Linear(32 * 4, 64)
        
        # Layer 4: Output Layer (2 outputs: 1 for Motor, 1 for Servo)
        self.fc2 = nn.Linear(64, 2)

    def forward(self, x):
        # Ensure input is shaped correctly: (Batch, Channels, Length) -> (Batch, 1, 8)
        if len(x.shape) == 2:
            x = x.unsqueeze(1) 
            
        # Layer 1 + Activation
        x = F.relu(self.conv1(x))
        
        # Layer 2 + Activation
        x = F.relu(self.conv2(x))
        
        # Flatten the tensor for the fully connected layers
        x = x.view(x.size(0), -1)
        
        # Layer 3 + Activation
        x = F.relu(self.fc1(x))
        
        # Layer 4 (Output)
        # Tanh restricts output between -1 and 1 (perfect for reverse/forward and left/right)
        x = torch.tanh(self.fc2(x))
        
        return x

# --- Example Usage ---
if __name__ == "__main__":
    # Instantiate the model
    model = RCCarCNN()
    print(model)
    
    # Simulate a single frame of sensor data (8 values)
    # e.g., [Lidar_F, Lidar_L, Lidar_R, Lidar_B, Ultrasound, Gyro_X, Gyro_Y, Gyro_Z]
    dummy_sensor_input = torch.randn(1, 8) 
    
    # Get control actions
    predictions = model(dummy_sensor_input)
    
    motor_throttle = predictions[0][0].item()
    steering_servo = predictions[0][1].item()
    
    print(f"\nOutputs (scaled between -1 and 1):")
    print(f"Motor Throttle: {motor_throttle:.4f}")
    print(f"Steering Servo: {steering_servo:.4f}")