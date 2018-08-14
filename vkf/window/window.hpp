#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <functional>
#include <list>
#include <vector>

namespace vkf {
class EventListener;

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

  double getDelta();

  bool getShouldClose() const;
  void pollEvents();

  std::vector<const char *> getVulkanExtensions();
  void createVulkanSurface(VkInstance instance, VkSurfaceKHR *surface);

  void addListener(EventListener *eventListener);
  void removeListener(EventListener *eventListener);

private:
  SDL_Window *window;

  bool shouldClose = false;

  int previousTime;
  int deltaTime;

  std::list<EventListener *> eventListeners;
};

class EventListener {
  friend class Window;

public:
  virtual ~EventListener() {
    if (this->window != nullptr) {
      this->window->removeListener(this);
    }
  };

  virtual void onResize(uint32_t width, uint32_t height){};

protected:
  class Window *window{nullptr};
};

} // namespace vkf
