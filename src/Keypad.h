#ifndef GUARD_KEYPAD_H
#define GUARD_KEYPAD_H

#include <SDL.h>

#include <unordered_map>

class Keypad {
 public:
  Keypad();

  bool processEvents();
  bool isKeyPressed(const uint8_t key_to_be_checked) const;

 private:
  static const std::unordered_map<SDL_Scancode, int> KEY_MAP;
  bool keypresses[12];
};

#endif
