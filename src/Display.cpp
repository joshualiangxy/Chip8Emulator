#include "Display.h"

#include <SDL.h>

const uint8_t VIDEO_WIDTH = 64;
const uint8_t VIDEO_HEIGHT = 32;

const uint32_t WHITE = 0xFFFFFFFF;
const uint32_t BLACK = 0x0;

Display::Display() { this->initDisplay(1); }

Display::Display(unsigned int scale_factor) { this->initDisplay(scale_factor); }

void Display::update() {
  SDL_UpdateTexture(this->texture.get(), nullptr, this->surface->pixels,
                    this->surface->pitch);
  SDL_RenderClear(this->renderer.get());
  SDL_RenderCopy(this->renderer.get(), this->texture.get(), nullptr, nullptr);
  SDL_RenderPresent(this->renderer.get());
}

void Display::initDisplay(unsigned int scale_factor) {
  SDL_Init(SDL_INIT_VIDEO);

  unsigned int scaled_width = VIDEO_WIDTH * scale_factor;
  unsigned int scaled_height = VIDEO_HEIGHT * scale_factor;

  SDL_Window* raw_window =
      SDL_CreateWindow("Chip8 Emulator", 100, 100, scaled_width, scaled_height,
                       SDL_WindowFlags::SDL_WINDOW_SHOWN);
  this->window.reset(raw_window, &SDL_DestroyWindow);

  SDL_Renderer* raw_renderer = SDL_CreateRenderer(
      this->window.get(), -1, SDL_RendererFlags::SDL_RENDERER_ACCELERATED);
  this->renderer.reset(raw_renderer, &SDL_DestroyRenderer);

  uint32_t rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xFF000000;
  gmask = 0x00FF0000;
  bmask = 0x0000FF00;
  amask = 0x000000FF;
#else
  rmask = 0x000000FF;
  gmask = 0x0000FF00;
  bmask = 0x00FF0000;
  amask = 0xFF000000;
#endif

  SDL_Surface* raw_surface = SDL_CreateRGBSurface(
      0, VIDEO_WIDTH, VIDEO_HEIGHT, 32, rmask, gmask, bmask, amask);
  this->surface.reset(raw_surface, &SDL_FreeSurface);

  SDL_Texture* raw_texture =
      SDL_CreateTextureFromSurface(this->renderer.get(), this->surface.get());
  this->texture.reset(raw_texture, &SDL_DestroyTexture);

  this->size = VIDEO_WIDTH * VIDEO_HEIGHT;
}

void Display::clear() {
  memset(this->surface->pixels, BLACK, VIDEO_WIDTH * VIDEO_HEIGHT);
}

size_t Display::getSize() { return this->size; }

bool Display::flipPixel(size_t index) {
  uint32_t pixel = ((uint32_t*)this->surface->pixels)[index];

  if (pixel == WHITE) {
    ((uint32_t*)this->surface->pixels)[index] = BLACK;
    return true;
  }

  ((uint32_t*)this->surface->pixels)[index] = WHITE;
  return false;
}
