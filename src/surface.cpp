#include "app.hpp"

using namespace app;

void App::createSurface() {
  if (glfwCreateWindowSurface(this->instance, this->window, nullptr,
                              &surface) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface");
  }
}
