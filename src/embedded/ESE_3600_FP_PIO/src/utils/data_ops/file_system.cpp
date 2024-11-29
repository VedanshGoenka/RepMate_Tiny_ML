#include "file_system.h"

bool allow_reformat = true; // Allow the file system to be reformatted during setup, if copy_files is false

bool file_system_setup()
{
  // First try to mount the filesystem
  if (!LittleFS.begin(allow_reformat && !copy_files))
  {
    Serial.println("Initial LittleFS Mount Failed");
  }

  // Handle formatting if needed
  if (force_reformat && !copy_files)
  {
    Serial.println("Forcing LittleFS format...");
    LittleFS.format();
    // After format, we must explicitly mount again
    if (!LittleFS.begin(false))
    {
      Serial.println("Failed to mount after forced format");
      return false;
    }
    Serial.println("LittleFS forcefully formatted and mounted");
    return true;
  }

  // If we reach here, try one more time with allow_reformat
  if (!LittleFS.begin(allow_reformat && !copy_files))
  {
    Serial.println("LittleFS Mount Failed");
    return false;
  }

  Serial.println("LittleFS mounted successfully");
  return true;
}

bool setup_folder_structure(uint8_t pin)
{
  // Input validation
  if (lift_class_folder_map.find(pin) == lift_class_folder_map.end())
  {
    Serial.printf("Invalid pin: %u\n", pin);
    return false;
  }

  String folder_path = "/" + lift_class_folder_map.at(pin); // Add leading slash

  // Create folder if it doesn't exist
  if (!LittleFS.exists(folder_path))
  {
    Serial.printf("Creating folder: %s\n", folder_path.c_str());
    if (!LittleFS.mkdir(folder_path))
    {
      Serial.printf("Failed to create folder: %s\n", folder_path.c_str());
      return false;
    }
  }
  return true;
}

int get_file_count(uint8_t pin)
{
  if (lift_class_folder_map.find(pin) == lift_class_folder_map.end())
  {
    Serial.printf("Invalid pin: %u\n", pin);
    return -1;
  }

  String folder_path = "/" + lift_class_folder_map.at(pin);
  File root = LittleFS.open(folder_path);
  if (!root)
  {
    Serial.printf("Failed to open directory: %s\n", folder_path.c_str());
    return -1;
  }

  int count = 0;
  File file = root.openNextFile();
  while (file)
  {
    count++;
    file.close();
    file = root.openNextFile();
  }
  root.close();
  return count;
}

String add_file_to_folder(uint8_t pin, String file_name, String extension)
{
  // Input validation
  if (file_name.isEmpty() || extension.isEmpty())
  {
    Serial.println("Invalid file name or extension");
    return "";
  }

  if (lift_class_folder_map.find(pin) == lift_class_folder_map.end())
  {
    Serial.printf("Invalid pin: %u\n", pin);
    return "";
  }

  // Ensure proper path format with leading slash
  String folder_path = "/" + lift_class_folder_map.at(pin);

  // Make sure folder exists
  if (!setup_folder_structure(pin))
  {
    return "";
  }

  int count = get_file_count(pin);
  if (count < 0)
    return "";

  String file_path = folder_path + "/" + file_name + String(count) + extension;

  File file = LittleFS.open(file_path, FILE_WRITE); // Use FILE_WRITE constant
  if (!file)
  {
    Serial.printf("Failed to create file: %s\n", file_path.c_str());
    return "";
  }
  file.close();
  Serial.printf("Created file: %s\n", file_path.c_str());
  return file_path;
}

bool write_to_file(const String &file_path, const String &file_content)
{
  if (file_path.isEmpty())
  {
    Serial.println("Invalid file path");
    return false;
  }

  File file = LittleFS.open(file_path, "w");
  if (!file)
  {
    Serial.printf("Failed to open file for writing: %s\n", file_path.c_str());
    return false;
  }

  size_t bytesWritten = file.print(file_content);
  file.close();

  if (bytesWritten != file_content.length())
  {
    Serial.println("Failed to write complete content to file");
    return false;
  }
  return true;
}
