#pragma once

#include "keycode.hpp"
#include "mousebutton.hpp"
#include "scancode.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <functional>
#include <list>
#include <vector>

namespace vkf {
class EventHandler;

class Window {
  typedef std::function<void(uint32_t width, uint32_t height)> OnResizeCallback;

public:
  Window(const char *title, int width, int height);
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  ~Window();

  uint32_t getWidth() const;
  uint32_t getHeight() const;

  void setRelativeMouse(bool relative);
  bool getRelativeMouse();
  void getRelativeMousePos(int *x, int *y);

  bool isKeyPressed(Scancode scancode);

  double getDelta();

  bool getShouldClose() const;
  void pollEvents();

  std::vector<const char *> getVulkanExtensions();
  void createVulkanSurface(VkInstance instance, VkSurfaceKHR *surface);

  void addHandler(EventHandler *eventHandler);
  void removeHandler(EventHandler *eventHandler);

private:
  SDL_Window *window;

  bool shouldClose = false;

  int previousTime;
  int deltaTime;

  std::list<EventHandler *> eventHandlers;
};
} // namespace vkf
