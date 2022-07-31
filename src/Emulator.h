#ifndef GUARD_EMULATOR_H
#define GUARD_EMULATOR_H

#include <cstdint>
#include <string>

#include "Display.h"
#include "Keypad.h"
#include "Processor.h"

class Emulator {
 public:
  Emulator(const std::string& rom_path);
  void start();

 private:
  Keypad keypad;
  Display display;
  Processor processor;
  uint32_t frames_per_second;
};

#endif
