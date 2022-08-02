#ifndef GUARD_KEYPAD_H
#define GUARD_KEYPAD_H

#include <SDL.h>

#include <unordered_map>
#include <unordered_set>

class Keypad {
 public:
  Keypad();

  bool processEvents();
  bool isKeyPressed(const uint8_t key_to_be_checked) const;
  int getKey() const;

 private:
  static const std::unordered_map<SDL_Scancode, uint8_t> KEY_MAP;

  std::unordered_set<uint8_t> pressed_keys;
};

#endif
