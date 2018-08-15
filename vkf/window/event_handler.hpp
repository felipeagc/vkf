#pragma once

#include "keycode.hpp"
#include "mousebutton.hpp"
#include "scancode.hpp"
#include "window.hpp"

namespace vkf {

class EventHandler {
  friend class Window;

public:
  virtual ~EventHandler() {
    if (this->window != nullptr) {
      this->window->removeHandler(this);
    }
  };

  virtual void onResize(uint32_t width, uint32_t height){};

  virtual void onKeyUp(Keycode key, bool repeat){};
  virtual void onKeyDown(Keycode key, bool repeat){};

  virtual void onButtonUp(MouseButton button, int x, int y){};
  virtual void onButtonDown(MouseButton button, int x, int y){};

protected:
  class Window *window{nullptr};
};
} // namespace vkf
