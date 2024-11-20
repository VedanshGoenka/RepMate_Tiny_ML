#pragma once

#include <LittleFS.h>
#include <map>
#include <Wire.h>
#include "data_collection.h"
#include "main.h"

extern const uint8_t pins[];

const std::map<uint8_t, String> lift_class_folder_map = {
    {pins[0], "p_f"},  // proper form
    {pins[1], "l_i"},  // lift instability
    {pins[2], "p_m"},  // partial motion
    {pins[3], "o_a"},  // off axis
    {pins[4], "s_w"}}; // swinging weight

bool file_system_setup();
bool setup_folder_structure(uint8_t pin);
int get_file_count(uint8_t pin);
String add_file_to_folder(uint8_t pin, String file_name, String extension);
bool write_to_file(const String &file_path, const String &file_content);
