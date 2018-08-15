#pragma once

#include <SDL2/SDL.h>

namespace vkf {
enum MouseButton {
  BUTTON_LEFT = SDL_BUTTON_LEFT,
  BUTTON_MIDDLE = SDL_BUTTON_MIDDLE,
  BUTTON_RIGHT = SDL_BUTTON_RIGHT,
  BUTTON_X1 = SDL_BUTTON_X1,
  BUTTON_X2 = SDL_BUTTON_X2,
};
} // namespace vkf
