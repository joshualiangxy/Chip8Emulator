#ifndef GUARD_KEYPAD_H
#define GUARD_KEYPAD_H

#include <SDL.h>

#include <unordered_map>

class Keypad {
 public:
  Keypad();

  bool processEvents();

 private:
  static const std::unordered_map<SDL_Scancode, int> KEY_MAP;
  bool keypresses[12];
};

#endif
