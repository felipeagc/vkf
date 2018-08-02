#pragma once

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include "validation.hpp"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>

namespace app {
#ifdef NDEBUG
const std::vector<const char *> REQUIRED_VALIDATION_LAYERS = {};
#else
const std::vector<const char *> REQUIRED_VALIDATION_LAYERS = {
    "VK_LAYER_LUNARG_standard_validation"};
#endif

const std::vector<const char *> REQUIRED_DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

class App {
public:
  App();
  virtual ~App();

  void run();

private:
  GLFWwindow *window;

  VkInstance instance = VK_NULL_HANDLE;
  VkDebugReportCallbackEXT callback = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  uint32_t graphicsQueueFamilyIndex;
  uint32_t presentQueueFamilyIndex;
  VkQueue graphicsQueue = VK_NULL_HANDLE;
  VkQueue presentQueue = VK_NULL_HANDLE;
  VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
  VkSemaphore renderingFinishedSemaphore = VK_NULL_HANDLE;

  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;

  VkCommandPool presentQueueCmdPool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> presentQueueCmdBuffers;

  bool canRender = false;

  void init();

  bool checkValidationLayerSupport();                // validation.cpp
  std::vector<const char *> getRequiredExtensions(); // extensions.cpp
  bool checkPhysicalDeviceProperties(
      VkPhysicalDevice physicalDevice,
      uint32_t *selectedGraphicsQueueFamilyIndex,
      uint32_t *selectedPresentQueueFamilyIndex); // device.cpp

  uint32_t
  getSwapchainNumImages(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
  VkSurfaceFormatKHR
  getSwapchainFormat(const std::vector<VkSurfaceFormatKHR> &formats);
  VkExtent2D
  getSwapchainExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
  VkImageUsageFlags
  getSwapchainUsageFlags(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
  VkSurfaceTransformFlagBitsKHR
  getSwapchainTransform(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
  VkPresentModeKHR
  getSwapchainPresentMode(const std::vector<VkPresentModeKHR> &presentModes);

  // Setup functions
  void createWindow();         // window.cpp
  void createInstance();       // instance.cpp
  void setupDebugCallback();   // validation.cpp
  void createSurface();        // surface.cpp
  void createDevice();         // device.cpp
  void getDeviceQueues();      // queue.cpp
  void createSemaphores();     // sync.cpp
  void createSwapchain();      // swapchain.cpp
  void createCommandBuffers(); // commands.cpp
  void recordCommandBuffers(); // commands.cpp

  void draw();     // drawing.cpp
  void onResize(); // drawing.cpp

  void destroyCommandPool(); // commands.cpp
};
} // namespace app
