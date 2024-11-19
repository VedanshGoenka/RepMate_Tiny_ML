#include "file_system.h"

bool file_system_setup()
{
  if (!LittleFS.begin())
  {
    Serial.println("Failed to mount LittleFS");
    return false;
  }
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

  String folder_path = lift_class_folder_map.at(pin);

  if (!LittleFS.exists(folder_path))
  {
    Serial.printf("Creating folder: %s\n", folder_path.c_str());
    return LittleFS.mkdir(folder_path);
  }

  Serial.printf("Folder already exists: %s\n", folder_path.c_str());
  return true;
}

int get_file_count(uint8_t pin)
{
  if (lift_class_folder_map.find(pin) == lift_class_folder_map.end())
  {
    Serial.printf("Invalid pin: %u\n", pin);
    return -1;
  }

  String folder_path = lift_class_folder_map.at(pin);
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

  String folder_path = lift_class_folder_map.at(pin);
  int count = get_file_count(pin);
  if (count < 0)
    return "";

  String file_path = folder_path + "/" + file_name + String(count) + extension;

  if (!LittleFS.exists(file_path))
  {
    File file = LittleFS.open(file_path, "w");
    if (!file)
    {
      Serial.printf("Failed to create file: %s\n", file_path.c_str());
      return "";
    }
    file.close();
    Serial.printf("Created file: %s\n", file_path.c_str());
  }
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
