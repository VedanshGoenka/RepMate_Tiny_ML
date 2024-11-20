#pragma once

#include <Arduino.h>
#include "utils/data_collection.h"
#include "utils/copy_files.h"

extern const bool copy_files;
extern const bool force_reformat;

extern const String lift_names[3];
extern String current_lift;

void setup();
void loop();