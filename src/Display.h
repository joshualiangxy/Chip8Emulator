#ifndef GUARD_DISPLAY_H
#define GUARD_DISPLAY_H

#include <SDL.h>

#include <memory>

class Display {
 public:
  Display();
  Display(unsigned int scale_factor);

  void update();
  void clear();
  size_t getSize();
  bool flipPixel(size_t index);

 private:
  std::shared_ptr<SDL_Window> window;
  std::shared_ptr<SDL_Renderer> renderer;
  std::shared_ptr<SDL_Surface> surface;
  std::shared_ptr<SDL_Texture> texture;
  size_t size;

  void initDisplay(unsigned int scale_factor);
};

#endif
