#pragma once

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include "validation.hpp"
#include "vertex.hpp"
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

const int MAX_FRAMES_IN_FLIGHT = 2;

class App {
public:
  App();
  virtual ~App();

  void run();

private:
  GLFWwindow *window;

  VkInstance instance{VK_NULL_HANDLE};
  VkDebugReportCallbackEXT callback{VK_NULL_HANDLE};
  VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
  VkDevice device{VK_NULL_HANDLE};

  uint32_t graphicsQueueFamilyIndex;
  uint32_t presentQueueFamilyIndex;
  VkQueue graphicsQueue{VK_NULL_HANDLE};
  VkQueue presentQueue{VK_NULL_HANDLE};

  std::vector<VkSemaphore> imageAvailableSemaphores{MAX_FRAMES_IN_FLIGHT};
  std::vector<VkSemaphore> renderingFinishedSemaphores{MAX_FRAMES_IN_FLIGHT};
  std::vector<VkFence> inFlightFences{MAX_FRAMES_IN_FLIGHT};
  int currentFrame = 0;

  VkSurfaceKHR surface{VK_NULL_HANDLE};

  VkSwapchainKHR swapchain = {VK_NULL_HANDLE};
  VkFormat swapchainImageFormat;
  VkExtent2D swapchainExtent;
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

  VkRenderPass renderPass;
  std::vector<VkFramebuffer> framebuffers{MAX_FRAMES_IN_FLIGHT};

  VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
  VkPipeline graphicsPipeline{VK_NULL_HANDLE};

  VkCommandPool graphicsCommandPool{VK_NULL_HANDLE};
  std::vector<VkCommandBuffer> graphicsCommandBuffers{MAX_FRAMES_IN_FLIGHT};

  // Local stuff
  // TODO: remove later
  VkBuffer vertexBuffer{VK_NULL_HANDLE};
  VkDeviceMemory vertexMemory{VK_NULL_HANDLE};

  VkImage textureImage{VK_NULL_HANDLE};
  VkImageView textureImageView{VK_NULL_HANDLE};
  VkSampler textureSampler{VK_NULL_HANDLE};
  VkDeviceMemory textureMemory{VK_NULL_HANDLE};

  VkBuffer stagingBuffer{VK_NULL_HANDLE};
  VkDeviceMemory stagingMemory{VK_NULL_HANDLE};

  VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
  VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
  VkDescriptorSet descriptorSet{VK_NULL_HANDLE};

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

  VkShaderModule createShaderModule(const char *filename); // shader.cpp

  VkPipelineLayout createPipelineLayout(); // pipeline.cpp

  VkBuffer createBuffer(
      size_t size,
      VkBufferUsageFlags usage); // vertex_buffer.cpp
  VkDeviceMemory allocateBufferMemory(
      VkBuffer &buffer,
      VkMemoryPropertyFlags properties); // vertex_buffer.cpp

  void createCommandPool(uint32_t queueFamilyIndex, VkCommandPool *pool);

  void allocateCommandBuffers(
      VkCommandPool pool, uint32_t count, VkCommandBuffer *commandBuffers);

  // Setup functions
  void createWindow();       // window.cpp
  void createInstance();     // instance.cpp
  void setupDebugCallback(); // validation.cpp
  void createSurface();      // surface.cpp
  void createDevice();       // device.cpp
  void getDeviceQueues();    // queue.cpp

  void createSyncObjects();         // sync.cpp
  void createSwapchain();           // swapchain.cpp
  void createSwapchainImageViews(); // swapchain.cpp
  void createCommandBuffers();      // commands.cpp

  void regenFramebuffer(
      VkFramebuffer &framebuffer,
      VkImageView imageView); // framebuffer.cpp

  void createRenderPass(); // renderpass.cpp
  void createPipeline();   // pipeline.cpp

  void
  createDescriptorSetLayout(VkDescriptorSetLayout *layout); // descriptors.cpp
  void
  createDescriptorPool(VkDescriptorPool *descriptorPool); // descriptors.cpp
  void allocateDescriptorSet(
      VkDescriptorPool descriptorPool,
      VkDescriptorSetLayout descriptorSetLayout,
      VkDescriptorSet *descriptorSet); // descriptors.cpp
  void updateDescriptorSetWithTexture(
      VkDescriptorSet descriptorSet,
      VkSampler sampler,
      VkImageView imageView); // descriptors.cpp

  void createStagingBuffer(); // vertex_buffer.cpp
  void createVertexBuffer(
      const std::vector<VertexData> &vertices); // vertex_buffer.cpp
  void
  copyVertexData(const std::vector<VertexData> &vertices); // vertex_buffer.cpp

  void
  createImage(uint32_t width, uint32_t height, VkImage *image); // image.cpp
  void allocateImageMemory(
      VkImage &image,
      VkMemoryPropertyFlags property,
      VkDeviceMemory *memory); // image.cpp
  void
  createImageView(const VkImage image, VkImageView *imageView); // image.cpp
  void createSampler(VkSampler *sampler);
  void createTexture(); // image.cpp
  void copyTextureData(
      unsigned char *textureData,
      size_t dataSize,
      uint32_t width,
      uint32_t height); // image.cpp

  void prepareFrame(int currentFrame, uint32_t imageIndex); // drawing.cpp
  void draw();                                              // drawing.cpp
  void onResize();                                          // drawing.cpp
  void destroyResizables();                                 // app.cpp
};
} // namespace app
