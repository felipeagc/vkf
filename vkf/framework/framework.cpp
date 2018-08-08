#include "framework.hpp"

using namespace vkf;

Framework::Framework(const char *title, int width, int height)
    : window(title, width, height), backend(window) {
}

Framework::~Framework() {
}

Window *Framework::getWindow() {
  return &this->window;
}

VulkanBackend *Framework::getBackend() {
  return &this->backend;
}
