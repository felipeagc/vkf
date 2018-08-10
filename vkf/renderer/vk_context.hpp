#pragma once

#include "../window/window.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <functional>
#include <iostream>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace vkf {

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

class VkContext : public EventListener {
public:
  VkContext(Window *window);
  VkContext(const VkContext &) = delete;
  VkContext &operator=(const VkContext &) = delete;
  ~VkContext();

  VmaAllocator getAllocator();
  VkDevice getDevice();
  VkRenderPass getRenderPass();
  VkQueue getGraphicsQueue();

  void useTransientCommandBuffer(std::function<void(VkCommandBuffer)> function);
  VkShaderModule createShaderModule(std::vector<char> code);

  void present(DrawFunction drawFunction);

  // EventHandler
  virtual void onResize(uint32_t width, uint32_t height) override;

private:
  Window *window{nullptr};

  VkInstance instance{VK_NULL_HANDLE};
  VkDebugReportCallbackEXT callback{VK_NULL_HANDLE};
  VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
  VkDevice device{VK_NULL_HANDLE};

  VmaAllocator allocator{VK_NULL_HANDLE};

  uint32_t graphicsQueueFamilyIndex;
  uint32_t presentQueueFamilyIndex;
  VkQueue graphicsQueue{VK_NULL_HANDLE};
  VkQueue presentQueue{VK_NULL_HANDLE};

  VkSurfaceKHR surface{VK_NULL_HANDLE};

  VkSwapchainKHR swapchain{VK_NULL_HANDLE};
  VkFormat swapchainImageFormat;
  VkExtent2D swapchainExtent;
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

  VkRenderPass renderPass;

  VkCommandPool graphicsCommandPool{VK_NULL_HANDLE};
  VkCommandPool transientCommandPool{VK_NULL_HANDLE};

  VkFormat depthImageFormat;
  struct FrameResources {
    VkImage depthImage;
    VmaAllocation depthImageAllocation;
    VkImageView depthImageView;

    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore renderingFinishedSemaphore{VK_NULL_HANDLE};
    VkFence fence{VK_NULL_HANDLE};

    VkFramebuffer framebuffer{VK_NULL_HANDLE};

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
  };

  std::vector<FrameResources> frameResources{MAX_FRAMES_IN_FLIGHT};

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

  // Creates the transient command pool
  void createTransientCommandPool();

  // Allocates the graphics command buffers used for drawing operations
  void allocateGraphicsCommandBuffers();

  // Creates the depth images and image views
  void createDepthResources();

  // Creates the renderpass
  void createRenderPass();

  // Deletes (if it exists) and recreates a framebuffer
  void regenFramebuffer(
      VkFramebuffer &framebuffer,
      VkImageView colorImageView,
      VkImageView depthImageView);

  // Destroys the resources that need to be destroyed when resizing the window
  void destroyResizables();
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
