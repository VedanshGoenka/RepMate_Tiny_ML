#include "copy_files.h"

bool copy_files_setup()
{
  // Initialize Serial communication with a longer timeout
  Serial.begin(115200);
  delay(1000);                                      // Give Serial more time to initialize
  Serial.println("\nStarting copy files setup..."); // Print immediately after delay

  // Mount filesystem without allowing reformatting
  if (!LittleFS.begin(false))
  {
    Serial.println("Failed to mount filesystem for copying");
    return false;
  }
  Serial.println("Filesystem mounted for copying");

  // Print folder structure to verify filesystem
  Serial.println("Available folders:");
  for (const auto &folder_pair : lift_class_folder_map)
  {
    Serial.printf("- %s\n", folder_pair.second.c_str());
  }

  return true;
}

void list_all_files()
{
  Serial.println("START_FILE_LIST");
  Serial.flush(); // Flush after marker
  delay(100);     // Give Python time to process

  for (const auto &folder_pair : lift_class_folder_map)
  {
    String folder_path = String("/") + folder_pair.second;
    File root = LittleFS.open(folder_path);
    if (!root || !root.isDirectory())
    {
      Serial.printf("Failed to open folder: %s\n", folder_path.c_str());
      continue;
    }

    File file = root.openNextFile();
    while (file)
    {
      if (!file.isDirectory())
      {
        Serial.printf("FILE:%s,%lu\n", file.path(), file.size()); // Added FILE: prefix
        Serial.flush();                                           // Flush after each file entry
        delay(20);                                                // Small delay between entries
      }
      file = root.openNextFile();
    }
    root.close();
  }

  Serial.println("END_FILE_LIST");
  Serial.flush();
  delay(100); // Give Python time to process
}

void read_file(const String &path)
{
  Serial.println("START_FILE_CONTENT"); // Added start marker
  Serial.flush();
  delay(10);

  File file = LittleFS.open(path, "r");
  if (file)
  {
    uint8_t buffer[4096]; // Increased to 4KB buffer
    while (file.available())
    {
      size_t bytesRead = file.read(buffer, 4096);
      Serial.write(buffer, bytesRead);
      Serial.flush();
      delay(25);
    }
    file.close();
  }

  Serial.println("\nEND_FILE_CONTENT"); // Added end marker
  Serial.flush();
  delay(100);
}

void handle_serial_commands()
{
  if (Serial.available())
  {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "list")
    {
      list_all_files();
    }
    else if (cmd.startsWith("read "))
    {
      String path = cmd.substring(5);
      read_file(path);
    }
  }
}
