#include "Emulator.h"

#include <cstdint>
#include <string>

static const uint16_t MILLISEC_IN_SEC = 1000;

Emulator::Emulator(const std::string& rom_path)
    : frames_per_second{60},
      keypad{},
      display{15},
      processor{rom_path, this->display, this->keypad} {}

void Emulator::start() {
  const uint32_t millisec_for_frame = MILLISEC_IN_SEC / this->frames_per_second;

  bool is_done = false;
  while (!is_done) {
    is_done = this->keypad.processEvents();
    this->processor.process();

    if (!this->processor.shouldUpdateDisplay()) {
      continue;
    }

    int start_time = SDL_GetTicks();

    this->display.update();

    int time_taken = SDL_GetTicks() - start_time;

    // if time taken is -ve, we have an overflow, so we continue
    if (time_taken < 0) continue;

    int sleep_time = millisec_for_frame - time_taken;
    if (sleep_time > 0) SDL_Delay(sleep_time);
  }
}
