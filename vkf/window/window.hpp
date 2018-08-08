#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <functional>
#include <vector>

namespace vkf {
class Window {
  typedef std::function<void(uint32_t width, uint32_t height)> OnResizeCallback;

public:
  Window(const char *title, int width, int height);
  ~Window();

  uint32_t getWidth() const;
  uint32_t getHeight() const;

  bool getShouldClose() const;
  void pollEvents();

  std::vector<const char*> getVulkanExtensions();
  void createVulkanSurface(VkInstance instance, VkSurfaceKHR *surface);

  void setOnResize(OnResizeCallback callback);

private:
  SDL_Window *window;

  bool shouldClose = false;

  OnResizeCallback onResize = [](uint32_t, uint32_t) {};
};
} // namespace vkf
