# RepMate: Tiny ML-Powered Workout Form Analysis

## Overview

RepMate is an intelligent workout assistant that uses machine learning to analyze and provide real-time feedback on weightlifting form. Using a combination of accelerometer and gyroscope data, the system can detect various types of lifts and classify the quality of form execution, providing immediate feedback to help users improve their workout technique.

## Authors

- Vedansh Goenka (vedanshg[at]seas.upenn.edu) [@vedanshgoenka](https://github.com/vedanshgoenka)
- Rohan Panday (rpanday[at]seas.upenn.edu) [@rp-seas](https://github.com/rp-seas)

## Final Report

[External Link](https://docs.google.com/document/d/14suUNHKm8XrC02RzAU51swQInUx69GyqMbsuufSj3Vk/edit?usp=sharing)

## Features

- Real-time motion analysis using MPU6050 sensor (accelerometer + gyroscope)
- Support for multiple lift types:
  - Dumbbell Curls (dC)
  - Bench Press (bP)
  - Dumbbell Flys (dF)
- Form classification categories:
  - Perfect Form (p_f)
  - Lift Instability (l_i)
  - Partial Motion (p_m)
  - Off-Axis Movement (o_a)
  - Swinging Weight (s_w)
  - No Lift (n_l)
- TensorFlow Lite model for efficient on-device inference
- JSON-based data collection for training and analysis
- Optimized TensorFlow Lite model for efficient on-device inference
- JSON-based data collection and processing pipeline
- Real-time feedback system with visual indicators
- Comprehensive data augmentation pipeline for robust model training

## Hardware Requirements

- Seeed XIAO ESP32S3 microcontroller
- MPU6050 IMU sensor
- Power source (USB or battery)
- Optional: LED indicators for visual feedback

## Software Dependencies

### Embedded System

- PlatformIO
- Arduino framework
- Required libraries:
  - Adafruit MPU6050
  - ArduinoJson
  - TensorFlowLite_ESP32
  - ESP32 Core

### Model Training Environment

- Python dependencies (see requirements.txt):
  - TensorFlow (with Metal support for MacOS)
  - NumPy
  - Pandas
  - Scikit-learn
  - Matplotlib
  - Seaborn
  - Jupyter
  - GraphViz

## Project Structure

```text
ESE_3600_Final_Project_RV/
├── src/
│   ├── embedded/                             # Embedded system code
│   │   └── ESE_3600_FP_PIO/                 # PlatformIO project
│   │       ├── src/                         # Source files
│   │       └── platformio.ini               # Project configuration
│   ├── model/                               # ML model development
│   │   ├── notebooks/                       # Training notebooks
│   │   │   ├── Initial_Model_TFLite.ipynb   # Model training and conversion
│   │   │   ├── plots/                       # Visualization outputs
│   │   │   └── anim/                        # Animation outputs
│   └── data/                            # Training data
│   └── ESE3600_input_format_schema.json     # Data format specification
```

## Setup Instructions

### 1. Hardware Setup

1. Connect the MPU6050 sensor to the ESP32S3:
   - VCC → 3.3V
   - GND → GND
   - SDA → GPIO 5 (SDA)
   - SCL → GPIO 6 (SCL)
2. Optional: Connect LED indicators to designated GPIO pins
3. Secure the sensor housing to the workout equipment

### 2. Software Setup

1. Install PlatformIO (VS Code extension recommended)
2. Clone the repository:

   ```bash
   git clone https://github.com/VedanshGoenka/RepMate_Tiny_ML.git
   cd RepMate_Tiny_ML
   ```

3. Install Python dependencies:

   ```bash
   pip install -r requirements.txt
   ```

### 3. Building and Flashing

1. Open the project in PlatformIO
2. Select the `esp32s3` environment in platformio.ini
3. Build and upload to your device
4. Monitor serial output for debugging (optional)

## Usage

The system operates in three modes, configured via flags in `main.cpp`:

1. **Data Collection Mode** (`collect_data = true`)
   - Captures raw sensor data for training
   - Saves data in JSON format
   - Supports multiple lift types:
     - Dumbbell Curls (dC)
     - Bench Press (bP)
     - Dumbbell Flys (dF)
   - Automatic file system formatting (controlled by `force_reformat`)

2. **Inference Mode** (`run_inference = true`)
   - Real-time form analysis with visual feedback
   - LED indicators on pins D0-D3, D6 for different form types:
     - D0: Perfect Form
     - D1: Lift Instability
     - D2: Off-Axis Movement
     - D3: Partial Motion
     - D6: Swinging Weight
   - Buzzer feedback for data collection and processing
   - BLE connectivity for mobile app integration (controlled by `ble_enabled`)
   - 2-second delay between predictions

3. **File Management Mode** (`copy_files = true`)
   - Safe file system access mode
   - Disables automatic formatting
   - Handles serial commands for file operations
   - Used for data transfer and management

4. **BLE Integration** (`ble_enabled = true`)
   - Real-time form classification updates
   - Mobile app connectivity
   - Wireless feedback system

5. **Audio Feedback** (`buzzer_enabled = true`)
   - Signals start/end of data collection
   - Provides auditory cues during operation
   - Multiple frequency patterns for different states

6. **System Configuration**
   - Configurable through boolean flags in `main.cpp`
   - Mutually exclusive operating modes
   - Optional features can be enabled/disabled independently

## Data Processing Pipeline

### 1. Raw Data Collection

Each sample contains time-series data from the MPU6050 sensor:

```json
{
    "t": timestamp,    // in milliseconds
    "aX": float,      // acceleration X
    "aY": float,      // acceleration Y
    "aZ": float,      // acceleration Z
    "gX": float,      // gyroscope X
    "gY": float,      // gyroscope Y
    "gZ": float,      // gyroscope Z
}
```

### 2. Data Pre-processing

1. **Sequence Processing**
   - Dynamic sequence length handling
   - Zero-padding for standardization
   - Moving average smoothing

2. **Feature Engineering**
   - Accelerometer and gyroscope data normalization
   - Derived features (jerk, rotation rate)
   - Time-domain feature extraction

3. **Data Augmentation**
   - Gaussian noise injection
   - Random scaling
   - Time warping
   - Axis rotation
   - Random sampling

### 3. Dataset Organization

- Training set: 70%
- Validation set: 20%
- Test set: 10%
- Stratified splitting
- Cross-validation implementation

## 4. Data Visualization

### Motion Animations

Watch how different form types appear in our sensor data:

#### Perfect Form

https://github.com/user-attachments/assets/0308fd6b-007b-4d54-b29b-a882042b10ca

*Animated visualization of perfect dumbbell curl form*

#### Common Form Issues

##### Lift Instability

https://github.com/user-attachments/assets/a70d95bf-0644-4a4b-90c3-d8a7e4b59dee

*Animated visualization of unstable lift patterns*

##### Swinging Weight


https://github.com/user-attachments/assets/c14b5c34-dd4d-47e2-8aed-605320c3cb4e


*Animated visualization of excessive swinging*

##### Partial Motion

https://github.com/user-attachments/assets/9cd6e893-a417-4ad1-a81a-49b55347cedb

*Animated visualization of incomplete range of motion*

##### No Lift

https://github.com/user-attachments/assets/eb1e538d-9987-4c2e-b513-913ba7daceda

*Animated Visualization of holding the dumbbell in the rest position* 

### Static Motion Patterns

The following 3D visualizations show the different motion patterns:

#### Perfect Form

![Perfect Form](src/model/notebooks/plots/Dumbbell%20Curl_Perfect%20Form_3d.png)
*3D visualization of accelerometer and gyroscope data for perfect form execution*

#### Common Form Issues

##### Lift Instability

![Lift Instability](src/model/notebooks/plots/Dumbbell%20Curl_Lift%20Instability_3d.png)
*3D visualization showing unstable motion patterns*

##### Swinging Weight

![Swinging Weight](src/model/notebooks/plots/Dumbbell%20Curl_Swinging%20Weight_3d.png)
*3D visualization demonstrating excessive swinging motion*

##### Partial Motion

![Partial Motion](src/model/notebooks/plots/Dumbbell%20Curl_Partial%20Motion_3d.png)
*3D visualization showing incomplete range of motion*

### Feature Analysis

Each form type has distinct characteristics in the sensor data:

![Perfect Form Features](src/model/notebooks/plots/Perfect%20Form_features.png)
*Feature analysis of perfect form execution*

![Lift Instability Features](src/model/notebooks/plots/Lift%20Instability_features.png)
*Feature analysis of unstable lift patterns*

These visualizations demonstrate the clear distinctions between different form types that our model learns to classify.

## Model Architecture

### Input Layer

- Shape: (200, 6) for time-series motion data

### Feature Extraction

1. **Conv1D Block 1**
   - 32 filters, kernel size 7
   - MaxPooling, BatchNorm, Dropout (0.2)

2. **Conv1D Block 2**
   - 64 filters, kernel size 5
   - MaxPooling, BatchNorm, Dropout (0.24)

3. **Conv1D Block 3**
   - 64 filters, kernel size 3
   - MaxPooling, BatchNorm, Dropout (0.3)

4. **Conv1D Block 4**
   - 64 filters, kernel size 3
   - GlobalAveragePooling, Dropout (0.34)

### Classification

1. Dense Layer (64 units, ReLU)
2. Dense Layer (32 units, ReLU, Dropout 0.4)
3. Output Layer (5 units, Softmax)

### Optimization

- AdamW optimizer
- Cosine decay learning rate
- Early stopping at 97% validation accuracy threshold
- Training completed in 16 epochs

## Performance Metrics

- Training accuracy: 99.17%
- Validation accuracy: 98.89%
- Training loss: 0.0206
- Validation loss: 0.1067
- Model size:
  - Original TensorFlow model: 295,501 bytes
  - TensorFlow Lite model: 85,320 bytes (reduced by 210,181 bytes)
- Perfect classification metrics:
  - Accuracy: 100%
  - Precision: 100%
  - Recall: 100%
  - F1 Score: 100%
- Inference time: ~74ms/step

## License

MIT License

## Acknowledgments

- ESE 3600 Course Staff
- TensorFlow Lite for Microcontrollers Team
- Seeed Studio Documentation
