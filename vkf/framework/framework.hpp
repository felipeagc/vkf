#pragma once

#include "../window/window.hpp"
#include "../renderer/vulkan_backend.hpp"

namespace vkf {
  class Framework {
  public:
    Framework(const char *title, int width, int height);
    ~Framework();

    Window* getWindow();
    VulkanBackend* getBackend();

  protected:
    Window window;
    VulkanBackend backend;
  };
}
