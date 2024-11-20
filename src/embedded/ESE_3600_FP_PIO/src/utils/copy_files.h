#pragma once

#include <LittleFS.h>
#include "file_system.h"

// Function to initialize connection to XIAO and mount filesystem
bool copy_files_setup();

// Function to list all files in the filesystem
void list_all_files();

// Function to handle serial commands for file operations
void handle_serial_commands();

// Function to read and send file contents over serial
void read_file(const String &path);
