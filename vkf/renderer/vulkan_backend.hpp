#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <functional>
#include <iostream>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace vkf {

class Window;

#ifdef NDEBUG
const std::vector<const char *> REQUIRED_VALIDATION_LAYERS = {};
#else
const std::vector<const char *> REQUIRED_VALIDATION_LAYERS = {
    "VK_LAYER_LUNARG_standard_validation",
};
#endif

const std::vector<const char *> REQUIRED_DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const int MAX_FRAMES_IN_FLIGHT = 2;

typedef std::function<void(VkCommandBuffer commandBuffer)> DrawFunction;

class VulkanBackend {
public:
  VulkanBackend(Window &window);
  ~VulkanBackend();

  // private:
  VkInstance instance{VK_NULL_HANDLE};
  VkDebugReportCallbackEXT callback{VK_NULL_HANDLE};
  VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
  VkDevice device{VK_NULL_HANDLE};

  VmaAllocator allocator{VK_NULL_HANDLE};

  uint32_t graphicsQueueFamilyIndex;
  uint32_t presentQueueFamilyIndex;
  VkQueue graphicsQueue{VK_NULL_HANDLE};
  VkQueue presentQueue{VK_NULL_HANDLE};

  std::vector<VkSemaphore> imageAvailableSemaphores{MAX_FRAMES_IN_FLIGHT};
  std::vector<VkSemaphore> renderingFinishedSemaphores{MAX_FRAMES_IN_FLIGHT};
  std::vector<VkFence> inFlightFences{MAX_FRAMES_IN_FLIGHT};

  VkSurfaceKHR surface{VK_NULL_HANDLE};

  VkSwapchainKHR swapchain = {VK_NULL_HANDLE};
  VkFormat swapchainImageFormat;
  VkExtent2D swapchainExtent;
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

  VkRenderPass renderPass;
  std::vector<VkFramebuffer> framebuffers{MAX_FRAMES_IN_FLIGHT};

  VkCommandPool graphicsCommandPool{VK_NULL_HANDLE};
  std::vector<VkCommandBuffer> graphicsCommandBuffers{MAX_FRAMES_IN_FLIGHT};

  int currentFrame = 0;

  // Checks if validation layers are supported
  bool checkValidationLayerSupport();

  // Gathers the required extensions into a vector
  std::vector<const char *>
  getRequiredExtensions(std::vector<const char *> sdlExtensions);

  // Checks if a physical device is suitable and gets its queue indices
  bool checkPhysicalDeviceProperties(
      VkPhysicalDevice physicalDevice,
      uint32_t *selectedGraphicsQueueFamilyIndex,
      uint32_t *selectedPresentQueueFamilyIndex);

  uint32_t
  getSwapchainNumImages(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
  VkSurfaceFormatKHR
  getSwapchainFormat(const std::vector<VkSurfaceFormatKHR> &formats);
  VkExtent2D getSwapchainExtent(
      uint32_t width,
      uint32_t height,
      const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
  VkImageUsageFlags
  getSwapchainUsageFlags(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
  VkSurfaceTransformFlagBitsKHR
  getSwapchainTransform(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
  VkPresentModeKHR
  getSwapchainPresentMode(const std::vector<VkPresentModeKHR> &presentModes);

  VkShaderModule createShaderModule(std::vector<char> code);

  // Intialization functions

  // Creates the vulkan instance
  void createInstance(std::vector<const char *> sdlExtensions);

  // Sets up the debug callback for the validation layers
  void setupDebugCallback();

  // Creates physical and logical devices
  void createDevice();

  // Retrieves the queues from the device
  void getDeviceQueues();

  // Sets up Vulkan Memory Allocator from AMD
  void setupMemoryAllocator();

  // Creates the semaphores and fences necessary for presentation
  void createSyncObjects();

  // Creates the swapchain
  void createSwapchain(uint32_t width, uint32_t height);

  // Creates the swapchain image views
  void createSwapchainImageViews();

  // Creates the graphics command pool
  void createGraphicsCommandPool();

  // Allocates the graphics command buffers used for drawing operations
  void allocateGraphicsCommandBuffers();

  // Creates the renderpass
  void createRenderPass();

  // Deletes (if it exists) and recreates a framebuffer
  void regenFramebuffer(VkFramebuffer &framebuffer, VkImageView imageView);

  // Resizing

  void destroyResizables();
  void onResize(uint32_t width, uint32_t height);

  // Drawing functions

  void present(uint32_t width, uint32_t height, DrawFunction drawFunction);
};
} // namespace vkf

VkResult CreateDebugReportCallbackEXT(
    VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugReportCallbackEXT *pCallback);

void DestroyDebugReportCallbackEXT(
    VkInstance instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks *pAllocator);
