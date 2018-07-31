#pragma once

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include "validation.hpp"

namespace app {
#ifdef NDEBUG
const std::vector<const char *> REQUIRED_VALIDATION_LAYERS = {};
#else
const std::vector<const char *> REQUIRED_VALIDATION_LAYERS = {
    "VK_LAYER_LUNARG_standard_validation"};
#endif

class App {
public:
  App();
  virtual ~App();

  void run();

private:
  VkInstance instance = VK_NULL_HANDLE;
  VkDebugReportCallbackEXT callback = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  uint32_t queueFamilyIndex;
  VkQueue queue = VK_NULL_HANDLE;

  void init();

  bool checkValidationLayerSupport();
  std::vector<const char*> getRequiredExtensions();

  // Setup functions
  void createInstance(); // instance.cpp
  void setupDebugCallback(); // validation.cpp
  void createDevice(); // device.cpp

  void cleanup();
};
} // namespace app
