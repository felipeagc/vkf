#include "window.hpp"
#include "event_handler.hpp"

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

void Window::setRelativeMouse(bool relative) {
  SDL_SetRelativeMouseMode(relative ? SDL_TRUE : SDL_FALSE);
}

bool Window::getRelativeMouse() {
  return SDL_GetRelativeMouseMode();
}

void Window::getRelativeMousePos(int *x, int *y) {
  SDL_GetRelativeMouseState(x, y);
}

bool Window::isKeyPressed(Scancode scancode) {
  const Uint8 *state = SDL_GetKeyboardState(NULL);
  return state[scancode] ? true : false;
}

double Window::getDelta() {
  return static_cast<double>(this->deltaTime) / 1000.0f;
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
    case SDL_KEYUP:
      for (EventHandler *handler : this->eventHandlers) {
        handler->onKeyUp(
            static_cast<Keycode>(event.key.keysym.sym),
            static_cast<bool>(event.key.repeat));
      }
      break;
    case SDL_KEYDOWN:
      for (EventHandler *handler : this->eventHandlers) {
        handler->onKeyDown(
            static_cast<Keycode>(event.key.keysym.sym),
            static_cast<bool>(event.key.repeat));
      }
      break;
    case SDL_MOUSEBUTTONUP:
      for (EventHandler *handler : this->eventHandlers) {
        handler->onButtonUp(
            static_cast<MouseButton>(event.button.button),
            event.button.x,
            event.button.y);
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      for (EventHandler *handler : this->eventHandlers) {
        handler->onButtonDown(
            static_cast<MouseButton>(event.button.button),
            event.button.x,
            event.button.y);
      }
      break;
    case SDL_WINDOWEVENT:
      switch (event.window.type) {
      case SDL_WINDOWEVENT_RESIZED:
        for (EventHandler *handler : this->eventHandlers) {
          handler->onResize(
              static_cast<uint32_t>(event.window.data1),
              static_cast<uint32_t>(event.window.data2));
        }
        break;
      }
      break;
    }
  }

  auto currentTime = SDL_GetTicks();

  this->deltaTime = currentTime - this->previousTime;

  this->previousTime = currentTime;
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

void Window::addHandler(EventHandler *eventHandler) {
  eventHandler->window = this;
  this->eventHandlers.push_back(eventHandler);
}

void Window::removeHandler(EventHandler *eventHandler) {
  this->eventHandlers.remove(eventHandler);
}
