#include "Keypad.h"

#include <SDL.h>

#include <algorithm>
#include <iterator>
#include <unordered_map>

const std::unordered_map<SDL_Scancode, uint8_t> Keypad::KEY_MAP = {
    {SDL_Scancode::SDL_SCANCODE_1, 0x1}, {SDL_Scancode::SDL_SCANCODE_2, 0x2},
    {SDL_Scancode::SDL_SCANCODE_3, 0x3}, {SDL_Scancode::SDL_SCANCODE_4, 0xC},
    {SDL_Scancode::SDL_SCANCODE_Q, 0x4}, {SDL_Scancode::SDL_SCANCODE_W, 0x5},
    {SDL_Scancode::SDL_SCANCODE_E, 0x6}, {SDL_Scancode::SDL_SCANCODE_R, 0xD},
    {SDL_Scancode::SDL_SCANCODE_A, 0x7}, {SDL_Scancode::SDL_SCANCODE_S, 0x8},
    {SDL_Scancode::SDL_SCANCODE_D, 0x9}, {SDL_Scancode::SDL_SCANCODE_F, 0xE},
    {SDL_Scancode::SDL_SCANCODE_Z, 0xA}, {SDL_Scancode::SDL_SCANCODE_X, 0x0},
    {SDL_Scancode::SDL_SCANCODE_C, 0xB}, {SDL_Scancode::SDL_SCANCODE_V, 0xF}};

Keypad::Keypad() {}

bool Keypad::processEvents() {
  SDL_Event sdl_event;

  while (SDL_PollEvent(&sdl_event)) {
    switch (sdl_event.type) {
      case SDL_EventType::SDL_QUIT:
        return true;

      case SDL_EventType::SDL_APP_WILLENTERBACKGROUND:
        SDL_WaitEvent(nullptr);
        break;

      case SDL_EventType::SDL_KEYDOWN:
      case SDL_EventType::SDL_KEYUP:
        SDL_Scancode key_scancode = sdl_event.key.keysym.scancode;
        if (Keypad::KEY_MAP.find(key_scancode) == Keypad::KEY_MAP.end()) {
          break;
        }

        uint8_t index = Keypad::KEY_MAP.at(key_scancode);

        bool is_key_pressed = sdl_event.type == SDL_EventType::SDL_KEYDOWN;
        if (is_key_pressed) {
          this->pressed_keys.insert(index);
        } else {
          this->pressed_keys.erase(index);
        }
        break;
    }
  }

  return false;
}

bool Keypad::isKeyPressed(const uint8_t key_to_be_checked) const {
  return this->pressed_keys.find(key_to_be_checked) !=
         this->pressed_keys.cend();
}

int Keypad::getKey() const {
  auto iter = this->pressed_keys.cbegin();
  if (iter == this->pressed_keys.cend()) {
    return -1;
  }

  return *iter;
}
