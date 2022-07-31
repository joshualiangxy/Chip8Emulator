#define SDL_MAIN_HANDLED

#include <string>

#include "Emulator.h"

int main(int argc, char* argv[]) {
  if (argc < 2) return -1;

  std::string rom_path = argv[1];
  Emulator emulator{rom_path};
  emulator.start();

  return 0;
}
