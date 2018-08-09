#include "window.hpp"

using namespace vkf;

Window::Window(const char *title, int width, int height) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);

  this->window = SDL_CreateWindow(
      title,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      width,
      height,
      SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
}

Window::~Window() {
  SDL_DestroyWindow(this->window);
  SDL_Quit();
}

uint32_t Window::getWidth() const {
  int width;
  SDL_GetWindowSize(window, &width, nullptr);
  return static_cast<uint32_t>(width);
}

uint32_t Window::getHeight() const {
  int height;
  SDL_GetWindowSize(window, nullptr, &height);
  return static_cast<uint32_t>(height);
}

bool Window::getShouldClose() const {
  return this->shouldClose;
}

void Window::pollEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      this->shouldClose = true;
      break;
    case SDL_WINDOWEVENT:
      switch (event.window.type) {
      case SDL_WINDOWEVENT_RESIZED:
        for (EventListener *listener : eventListeners) {
          listener->onResize(
              static_cast<uint32_t>(event.window.data1),
              static_cast<uint32_t>(event.window.data2));
        }
        break;
      }
      break;
    }
  }
}

std::vector<const char *> Window::getVulkanExtensions() {
  uint32_t sdlExtensionCount = 0;
  SDL_Vulkan_GetInstanceExtensions(this->window, &sdlExtensionCount, nullptr);
  std::vector<const char *> sdlExtensions(sdlExtensionCount);
  SDL_Vulkan_GetInstanceExtensions(
      this->window, &sdlExtensionCount, sdlExtensions.data());
  return sdlExtensions;
}

void Window::createVulkanSurface(VkInstance instance, VkSurfaceKHR *surface) {
  if (!SDL_Vulkan_CreateSurface(window, instance, surface)) {
    throw std::runtime_error("Failed to create window surface");
  }
}

void Window::addListener(EventListener *eventListener) {
  eventListener->window = this;
  this->eventListeners.push_back(eventListener);
}

void Window::removeListener(EventListener *eventListener) {
  this->eventListeners.remove(eventListener);
}
