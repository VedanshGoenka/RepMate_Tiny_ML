#pragma once

#include <map>
#include <Wire.h>
#include <LittleFS.h>
#include "data_collection.h"

extern const uint8_t pins[];

const std::map<uint8_t, String> lift_class_folder_map = {
    {pins[0], "/proper_form"},
    {pins[1], "/lift_instability"},
    {pins[2], "/partial_motion"},
    {pins[3], "/off_axis"},
    {pins[4], "/swinging_weight"}};

bool file_system_setup();
bool setup_folder_structure(uint8_t pin);
int get_file_count(uint8_t pin);
String add_file_to_folder(uint8_t pin, String file_name, String extension);
bool write_to_file(String file_path, String file_content);
