# RepMate: AI-Powered Workout Form Analysis

## Overview

RepMate is an intelligent workout assistant that uses machine learning to analyze and provide real-time feedback on weightlifting form. Using a combination of accelerometer and gyroscope data, the system can detect various types of lifts and classify the quality of form execution.

## Authors

- Vedansh Goenka (vedanshg[at]seas.upenn.edu) [@vedanshgoenka](https://github.com/vedanshgoenka)
- Rohan Panday (rpanday[at]seas.upenn.edu) [@rp-seas](https://github.com/rp-seas)

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
- TensorFlow Lite model for efficient on-device inference
- JSON-based data collection for training and analysis

## Hardware Requirements

- Seeed XIAO ESP32S3 microcontroller
- MPU6050 IMU sensor
- Power source (USB or battery)

## Software Dependencies

### Embedded System

- PlatformIO
- Arduino framework
- Required libraries:
  - Adafruit MPU6050
  - ArduinoJson
  - TensorFlowLite_ESP32

### Model Training Environment

- Python dependencies (see requirements.txt):
  - TensorFlow
  - NumPy
  - Pandas
  - Scikit-learn
  - Matplotlib
  - Seaborn

## Project Structure

```text
ESE_3600_Final_Project_RV/
├── src/
│   ├── embedded/                             # Embedded system code
│   │   └── ESE_3600_FP_PIO/                  # PlatformIO project
│   │       ├── src/                          # Source files
│   │       └── platformio.ini                # Project configuration
│   ├── model/                                # ML model development
│   │   └── notebooks/                        # Training notebooks
│   │   └── ESE3600_input_format_schema.json  # Data format specification
│   └── requirements.txt                      # Python dependencies
```

## Setup Instructions

### 1. Hardware Setup (WIP ROHAN)

1. Connect the MPU6050 sensor to the ESP32S3:
   - VCC → 3.3V
   - GND → GND
   - SDA → SDA
   - SCL → SCL

### 2. Software Setup (WIP VEDANSH)

1. Install PlatformIO (VS Code extension recommended)
2. Clone the repository:

   ```bash
   git clone [repository-url]
   cd ESE_3600_Final_Project_RV
   ```

3. Install Python dependencies:

   ```bash
   pip install -r requirements.txt
   ```

### 3. Building and Flashing

1. Open the project in PlatformIO
2. Select the appropriate environment in platformio.ini
3. Build and upload to your device

## Usage

The system operates in three modes (configured in main.cpp):

1. **Data Collection Mode** (`collect_data = true`)
   - Captures raw sensor data for training
   - Saves data in JSON format
   - Use buttons to classify different forms

2. **Inference Mode** (`run_inference = true`)
   - Real-time form analysis
   - Provides immediate feedback on lift quality

3. **File Copy Mode** (`copy_files = true`)
   - Utility mode for data transfer
   - Manages file system operations

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
    "gZ": float       // gyroscope Z
}
```

### 2. Data Pre-processing

1. **Dataset Extension**
   - Extends shorter sequences to target length (1000 samples)
   - Adds zero-padding at head and tail of sequence, evenly distributed

2. **Downsampling**
   - Reduces sequence length to 200 samples
   - Uses averaging window technique, with window size of 5.
   - Supports random sampling for augmentation.

3. **Normalization**
   - Per-sample normalization
   - Separate normalization for accelerometer and gyroscope data
   - Z-score normalization (mean=0, std=1)

4. **Data Augmentation**
   - **Augmentations**:
     - Random noise injection
     - Random value dropping
     - Magnitude scaling
     - Paired inversion of axes
   - **Applying Augmentations**:
     - Original sample plus 1 to 3 augmentation samples
     - 1 to 2 augmentations are randomly selected for each sample

### 3. Dataset Split

- Training set: 70% (augmented)
- Validation set: 20%
- Test set: 10% (augmented)
- Stratified split to ensure class distribution

## Model Architecture

The neural network uses a 1D convolutional architecture optimized for time-series motion data:

### Input Layer

- Shape: (200, 6) representing 200 time steps of 6 features (3-axis accelerometer + 3-axis gyroscope)

### Feature Extraction Layers

1. **First Conv1D Block**
   - Conv1D: 32 filters, kernel size 7, same padding
   - ReLU activation
   - MaxPooling1D: pool size 2
   - BatchNormalization
   - Dropout: 0.2

2. **Second Conv1D Block**
   - Conv1D: 64 filters, kernel size 5, same padding
   - ReLU activation
   - MaxPooling1D: pool size 2
   - BatchNormalization
   - Dropout: 0.24

3. **Third Conv1D Block**
   - Conv1D: 64 filters, kernel size 3, same padding
   - ReLU activation
   - MaxPooling1D: pool size 2
   - BatchNormalization
   - Dropout: 0.3

4. **Fourth Conv1D Block**
   - Conv1D: 64 filters, kernel size 3, same padding
   - ReLU activation
   - GlobalAveragePooling1D
   - Dropout: 0.34

### Classification Layers

1. **First Dense Block**
   - Dense: 64 units
   - ReLU activation

2. **Second Dense Block**
   - Dense: 32 units
   - ReLU activation
   - Dropout: 0.4

3. **Output Layer**
   - Dense: 5 units (number of form classes)
   - Softmax activation

### Model Optimization

- **Learning Rate Schedule**: Cosine decay with warmup
- **Optimizer**: AdamW with weight decay
- **Loss Function**: Categorical Cross-entropy
- **Early Stopping**: Validation accuracy threshold of 96%

### Deployment Optimization

1. **TFLite Conversion**
   - Float32 to Int8 quantization
   - Representative dataset used for calibration
   - ~70% reduction in model size

2. **Performance Metrics**
   - Accuracy maintained after quantization
   - Inference time optimized for real-time processing
   - Memory footprint suitable for ESP32S3

## License

TBD

## Acknowledgments

- ESE 3600 Course Staff
