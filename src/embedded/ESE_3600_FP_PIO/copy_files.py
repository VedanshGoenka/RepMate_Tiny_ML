import serial
import os
import time
import glob
from datetime import datetime


def find_serial_port():
    """Find the correct serial port for XIAO ESP32S3 on macOS"""
    # Common patterns for XIAO ESP32S3 on macOS
    patterns = [
        "/dev/tty.usbmodem*",  # Common macOS pattern for USB CDC devices
        "/dev/tty.wchusbserial*",  # Alternative pattern
        "/dev/tty.SLAB_USBtoUART*",  # Another possible pattern
    ]

    for pattern in patterns:
        ports = glob.glob(pattern)
        if ports:
            return ports[0]

    raise Exception("No XIAO ESP32S3 device found! Please check connection.")


def create_folder(path):
    if not os.path.exists(path):
        os.makedirs(path)


def copy_files():
    try:
        # Find and configure serial connection
        port = find_serial_port()
        print(f"Found device at: {port}")
        ser = serial.Serial(port, 115200, timeout=2)
        time.sleep(2)

        # Flush buffers
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        # Get file list first
        print("Requesting file list...")
        ser.write(b"list\n")
        ser.flush()

        files_to_copy = []
        # Wait for start marker
        while True:
            line = ser.readline().decode().strip()
            if line == "START_FILE_LIST":
                break

        # Collect file list
        while True:
            line = ser.readline().decode().strip()
            if line == "END_FILE_LIST":
                break
            if line.startswith("FILE:"):  # Check for FILE: prefix
                try:
                    file_info = line[5:]  # Remove FILE: prefix
                    file_path, size = file_info.split(",")
                    files_to_copy.append((file_path, int(size)))
                except Exception as e:
                    print(f"Error parsing file info: {line} - {str(e)}")

        # Only create folder if there are files to copy
        if not files_to_copy:
            print("No files found to copy!")
            ser.close()
            return

        # Create timestamp folder only if we have files
        timestamp = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
        base_path = os.path.join("data", timestamp)
        create_folder(base_path)
        print(f"Created folder: {base_path}")

        # Copy each file
        for file_path, size in files_to_copy:
            try:
                print(f"\nRequesting: {file_path}")
                dest_path = os.path.join(base_path, file_path[1:])
                os.makedirs(os.path.dirname(dest_path), exist_ok=True)

                ser.write(f"read {file_path}\n".encode())
                ser.flush()

                # Wait for start marker
                while True:
                    line = ser.readline().decode().strip()
                    if line == "START_FILE_CONTENT":
                        break

                # Read file content
                with open(dest_path, "wb") as f:
                    bytes_read = 0
                    while bytes_read < size:
                        data = ser.read(min(4096, size - bytes_read))
                        if not data:
                            break
                        f.write(data)
                        bytes_read += len(data)
                        print(f"Progress: {bytes_read}/{size} bytes", end="\r")

                # Wait for end marker
                while True:
                    line = ser.readline().decode().strip()
                    if line == "END_FILE_CONTENT":
                        break

                print(f"\n✓ Copied: {file_path}")

            except Exception as e:
                print(f"✗ Failed to copy {file_path}: {str(e)}")

        ser.close()
        print(f"\nAll files copied to: {base_path}")

    except Exception as e:
        print(f"Error: {str(e)}")
        if "ser" in locals():
            ser.close()


if __name__ == "__main__":
    copy_files()
