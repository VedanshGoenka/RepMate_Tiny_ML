#include "main.h"

const bool copy_files = true;      // If true, SD card sets formatting flags to NEVER format
const bool force_reformat = false; // If true, the file system will be reformatted during setup

const String lift_names[3] = {"dC", "bP", "dF"}; // dumbbell curl, bench press, dumbbell flys
String current_lift = lift_names[0];

void setup()
{
  if (copy_files)
  {
    copy_files_setup();
  }
  else
  {
    data_collection_setup();
  }
}

void loop()
{
  if (copy_files)
  {
    handle_serial_commands();
  }
  else
  {
    data_collection_loop();
  }
}
