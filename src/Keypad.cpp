#include "Keypad.h"

#include <SDL.h>

#include <algorithm>
#include <iterator>
#include <unordered_map>

const std::unordered_map<SDL_Scancode, int> Keypad::KEY_MAP = {
    {SDL_Scancode::SDL_SCANCODE_1, 0},  {SDL_Scancode::SDL_SCANCODE_2, 1},
    {SDL_Scancode::SDL_SCANCODE_3, 2},  {SDL_Scancode::SDL_SCANCODE_Q, 3},
    {SDL_Scancode::SDL_SCANCODE_W, 4},  {SDL_Scancode::SDL_SCANCODE_E, 5},
    {SDL_Scancode::SDL_SCANCODE_A, 6},  {SDL_Scancode::SDL_SCANCODE_S, 7},
    {SDL_Scancode::SDL_SCANCODE_D, 8},  {SDL_Scancode::SDL_SCANCODE_Z, 9},
    {SDL_Scancode::SDL_SCANCODE_X, 10}, {SDL_Scancode::SDL_SCANCODE_C, 11}};

Keypad::Keypad() {
  std::fill(std::begin(this->keypresses), std::end(this->keypresses), false);
}

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

        int index = Keypad::KEY_MAP.at(key_scancode);
        this->keypresses[index] = sdl_event.type == SDL_EventType::SDL_KEYDOWN;
        break;
    }
  }

  return false;
}
