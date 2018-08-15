#include "vk_context.hpp"
#include "../window/window.hpp"
#include <cstring>

using namespace vkf;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char *layerPrefix,
    const char *msg,
    void *userData) {

  std::cerr << "Validation layer: " << msg << std::endl;

  return VK_FALSE;
}

VkContext::VkContext(Window *window) : window(window) {
  this->createInstance(window->getVulkanExtensions());
#ifndef NDEBUG
  this->setupDebugCallback();
#endif
  window->createVulkanSurface(this->instance, &this->surface);
  this->createDevice();
  this->getDeviceQueues();
  this->setupMemoryAllocator();

  this->createSyncObjects();

  this->createSwapchain(window->getWidth(), window->getHeight());
  this->createSwapchainImageViews();

  this->createGraphicsCommandPool();
  this->createTransientCommandPool();
  this->allocateGraphicsCommandBuffers();

  this->createDepthResources();

  this->createRenderPass();

  this->window->addHandler(this);
}

VkContext::~VkContext() {
  if (this->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->device);

    this->destroyResizables();

    if (this->transientCommandPool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(this->device, this->transientCommandPool, nullptr);
      this->transientCommandPool = VK_NULL_HANDLE;
    }

    if (this->graphicsCommandPool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(this->device, this->graphicsCommandPool, nullptr);
      this->graphicsCommandPool = VK_NULL_HANDLE;
    }

    for (const auto &resources : this->frameResources) {
      if (resources.framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(this->device, resources.framebuffer, nullptr);
      }

      if (resources.fence != VK_NULL_HANDLE) {
        vkDestroyFence(this->device, resources.fence, nullptr);
      }

      if (resources.renderingFinishedSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(
            this->device, resources.renderingFinishedSemaphore, nullptr);
      }

      if (resources.imageAvailableSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(
            this->device, resources.imageAvailableSemaphore, nullptr);
      }
    }

    for (const auto &imageView : this->swapchainImageViews) {
      if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(this->device, imageView, nullptr);
      }
    }

    if (this->swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
    }

    if (this->allocator != VK_NULL_HANDLE) {
      vmaDestroyAllocator(this->allocator);
    }

    vkDestroyDevice(this->device, nullptr);
  }

  if (this->surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
  }

#ifndef NDEBUG
  if (this->callback != VK_NULL_HANDLE) {
    DestroyDebugReportCallbackEXT(this->instance, this->callback, nullptr);
  }
#endif

  if (this->instance != VK_NULL_HANDLE) {
    vkDestroyInstance(this->instance, nullptr);
  }
}

VmaAllocator VkContext::getAllocator() {
  return this->allocator;
}

VkDevice VkContext::getDevice() {
  return this->device;
}

VkRenderPass VkContext::getRenderPass() {
  return this->renderPass;
}

VkQueue VkContext::getGraphicsQueue() {
  return this->graphicsQueue;
}

void VkContext::useTransientCommandBuffer(
    std::function<void(VkCommandBuffer)> function) {
  VkCommandBuffer commandBuffer{VK_NULL_HANDLE};

  VkCommandBufferAllocateInfo allocateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = this->transientCommandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  if (vkAllocateCommandBuffers(this->device, &allocateInfo, &commandBuffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate transient command buffer");
  }

  function(commandBuffer);

  vkFreeCommandBuffers(
      this->device, this->transientCommandPool, 1, &commandBuffer);
}

bool VkContext::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : REQUIRED_VALIDATION_LAYERS) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerProperties.layerName, layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

std::vector<const char *>
VkContext::getRequiredExtensions(std::vector<const char *> sdlExtensions) {
  std::vector<const char *> extensions{sdlExtensions};

#ifndef NDEBUG
  extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

  return extensions;
}

bool VkContext::checkPhysicalDeviceProperties(
    VkPhysicalDevice physicalDevice,
    uint32_t *selectedGraphicsQueueFamilyIndex,
    uint32_t *selectedPresentQueueFamilyIndex) {
  uint32_t extensionCount = 0;
  if (vkEnumerateDeviceExtensionProperties(
          physicalDevice, nullptr, &extensionCount, nullptr) != VK_SUCCESS ||
      extensionCount == 0) {
    std::cout << "Error occurred during physical device " << physicalDevice
              << " extension enumeration" << std::endl;
    return false;
  }

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  if (vkEnumerateDeviceExtensionProperties(
          physicalDevice,
          nullptr,
          &extensionCount,
          availableExtensions.data()) != VK_SUCCESS) {
    std::cout << "Error occurred during physical device " << physicalDevice
              << " extension enumeration" << std::endl;
    return false;
  }

  for (const auto &requiredExtension : REQUIRED_DEVICE_EXTENSIONS) {
    bool found = false;
    for (const auto &extension : availableExtensions) {
      if (strcmp(requiredExtension, extension.extensionName) == 0) {
        found = true;
      }
    }

    if (!found) {
      std::cout << "Physical device " << physicalDevice
                << " doesn't support extension named \"" << requiredExtension
                << "\"" << std::endl;
      return false;
    }
  }

  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceFeatures deviceFeatures;

  vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
  vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

  uint32_t majorVersion = VK_VERSION_MAJOR(deviceProperties.apiVersion);
  // uint32_t minorVersion = VK_VERSION_MINOR(deviceProperties.apiVersion);
  // uint32_t patchVersion = VK_VERSION_PATCH(deviceProperties.apiVersion);

  if (majorVersion < 1 && deviceProperties.limits.maxImageDimension2D < 4096) {
    std::cout << "Physical device " << physicalDevice
              << " doesn't support required parameters!" << std::endl;
    return false;
  }

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
      physicalDevice, &queueFamilyCount, nullptr);

  if (queueFamilyCount == 0) {
    std::cout << "Physical device " << physicalDevice
              << " doesn't have any queue families" << std::endl;
    return false;
  }

  std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
  std::vector<VkBool32> queuePresentSupport(queueFamilyCount);

  vkGetPhysicalDeviceQueueFamilyProperties(
      physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

  uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
  uint32_t presentQueueFamilyIndex = UINT32_MAX;

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    vkGetPhysicalDeviceSurfaceSupportKHR(
        physicalDevice, i, this->surface, &queuePresentSupport[i]);

    if (queueFamilyProperties[i].queueCount > 0 &&
        queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      if (graphicsQueueFamilyIndex == UINT32_MAX) {
        graphicsQueueFamilyIndex = i;
      }

      if (queuePresentSupport[i]) {
        *selectedGraphicsQueueFamilyIndex = i;
        *selectedPresentQueueFamilyIndex = i;
        return true;
      }
    }
  }

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    if (queuePresentSupport[i]) {
      presentQueueFamilyIndex = i;
      break;
    }
  }

  if (graphicsQueueFamilyIndex == UINT32_MAX ||
      presentQueueFamilyIndex == UINT32_MAX) {
    std::cout << "Could not find queue family with requested properties on "
                 "physical device "
              << physicalDevice << std::endl;
    return false;
  }

  *selectedGraphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
  *selectedPresentQueueFamilyIndex = presentQueueFamilyIndex;

  return true;
}

uint32_t VkContext::getSwapchainNumImages(
    const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
  uint32_t imageCount = surfaceCapabilities.minImageCount + 1;

  if (surfaceCapabilities.maxImageCount > 0 &&
      imageCount > surfaceCapabilities.maxImageCount) {
    imageCount = surfaceCapabilities.maxImageCount;
  }

  return imageCount;
}

VkSurfaceFormatKHR
VkContext::getSwapchainFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
  if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
    return {VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
  }

  for (const auto &format : formats) {
    if (format.format == VK_FORMAT_R8G8B8A8_UNORM) {
      return format;
    }
  }

  return formats[0];
}

VkExtent2D VkContext::getSwapchainExtent(
    uint32_t width,
    uint32_t height,
    const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
  if (surfaceCapabilities.currentExtent.width == static_cast<uint32_t>(-1)) {
    VkExtent2D swapchainExtent = {width, height};
    if (swapchainExtent.width < surfaceCapabilities.minImageExtent.width) {
      swapchainExtent.width = surfaceCapabilities.minImageExtent.width;
    }

    if (swapchainExtent.height < surfaceCapabilities.minImageExtent.height) {
      swapchainExtent.height = surfaceCapabilities.minImageExtent.height;
    }

    if (swapchainExtent.width > surfaceCapabilities.maxImageExtent.width) {
      swapchainExtent.width = surfaceCapabilities.maxImageExtent.width;
    }

    if (swapchainExtent.height > surfaceCapabilities.maxImageExtent.height) {
      swapchainExtent.height = surfaceCapabilities.maxImageExtent.height;
    }

    return swapchainExtent;
  }

  return surfaceCapabilities.currentExtent;
}

VkImageUsageFlags VkContext::getSwapchainUsageFlags(
    const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
  if (surfaceCapabilities.supportedUsageFlags &
      VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
    return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
           VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }
  std::cout << "VK_IMAGE_USAGE_TRANSFER_DST image usage is not supported by "
               "the swap chain!"
            << std::endl
            << "Supported swap chain's image usages include:" << std::endl
            << (surfaceCapabilities.supportedUsageFlags &
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                    ? "    VK_IMAGE_USAGE_TRANSFER_SRC\n"
                    : "")
            << (surfaceCapabilities.supportedUsageFlags &
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT
                    ? "    VK_IMAGE_USAGE_TRANSFER_DST\n"
                    : "")
            << (surfaceCapabilities.supportedUsageFlags &
                        VK_IMAGE_USAGE_SAMPLED_BIT
                    ? "    VK_IMAGE_USAGE_SAMPLED\n"
                    : "")
            << (surfaceCapabilities.supportedUsageFlags &
                        VK_IMAGE_USAGE_STORAGE_BIT
                    ? "    VK_IMAGE_USAGE_STORAGE\n"
                    : "")
            << (surfaceCapabilities.supportedUsageFlags &
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                    ? "    VK_IMAGE_USAGE_COLOR_ATTACHMENT\n"
                    : "")
            << (surfaceCapabilities.supportedUsageFlags &
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                    ? "    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT\n"
                    : "")
            << (surfaceCapabilities.supportedUsageFlags &
                        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
                    ? "    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT\n"
                    : "")
            << (surfaceCapabilities.supportedUsageFlags &
                        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
                    ? "    VK_IMAGE_USAGE_INPUT_ATTACHMENT"
                    : "")
            << std::endl;

  return static_cast<VkImageUsageFlags>(-1);
}

VkSurfaceTransformFlagBitsKHR VkContext::getSwapchainTransform(
    const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
  if (surfaceCapabilities.supportedTransforms &
      VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
    return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  } else {
    return surfaceCapabilities.currentTransform;
  }
}

VkPresentModeKHR VkContext::getSwapchainPresentMode(
    const std::vector<VkPresentModeKHR> &presentModes) {
  for (const auto &presentMode : presentModes) {
    if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      return presentMode;
    }
  }

  for (const auto &presentMode : presentModes) {
    if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return presentMode;
    }
  }

  for (const auto &presentMode : presentModes) {
    if (presentMode == VK_PRESENT_MODE_FIFO_KHR) {
      return presentMode;
    }
  }

  std::cout << "FIFO present mode is not supported by the swap chain!"
            << std::endl;

  return static_cast<VkPresentModeKHR>(-1);
}

VkShaderModule VkContext::createShaderModule(std::vector<char> code) {
  if (code.size() == 0) {
    throw std::runtime_error("Shader code loaded with size 0");
  }

  VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.pNext = nullptr;
  shaderModuleCreateInfo.flags = 0;
  shaderModuleCreateInfo.codeSize = code.size();
  shaderModuleCreateInfo.pCode =
      reinterpret_cast<const uint32_t *>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(
          this->device, &shaderModuleCreateInfo, nullptr, &shaderModule) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module");
  }

  return shaderModule;
}

void VkContext::createInstance(std::vector<const char *> sdlExtensions) {
#ifndef NDEBUG
  if (!checkValidationLayerSupport()) {
    throw std::runtime_error("Validation layers requested, but not available");
  }
#endif

  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = "App",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_0,
  };

  VkInstanceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &appInfo,
  };

#ifdef NDEBUG
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = nullptr;
#else
  createInfo.enabledLayerCount =
      static_cast<uint32_t>(REQUIRED_VALIDATION_LAYERS.size());
  createInfo.ppEnabledLayerNames = REQUIRED_VALIDATION_LAYERS.data();
#endif

  auto extensions = this->getRequiredExtensions(sdlExtensions);
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create vulkan instance");
  }
}

void VkContext::setupDebugCallback() {
  VkDebugReportCallbackCreateInfoEXT createInfo = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
      .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
      .pfnCallback = debugCallback,
  };

  if (CreateDebugReportCallbackEXT(
          this->instance, &createInfo, nullptr, &this->callback) !=
      VK_SUCCESS) {

    throw std::runtime_error("Failed to setup debug callback");
  }
}

void VkContext::createDevice() {
  uint32_t deviceCount = 0;
  if (vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr) !=
          VK_SUCCESS ||
      deviceCount == 0) {
    throw std::runtime_error(
        "Error occurred during physical device enumeration");
  }

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  if (vkEnumeratePhysicalDevices(
          this->instance, &deviceCount, physicalDevices.data()) != VK_SUCCESS) {
    throw std::runtime_error(
        "Error occurred during physical device enumeration");
  }

  uint32_t selectedGraphicsQueueFamilyIndex = UINT32_MAX;
  uint32_t selectedPresentQueueFamilyIndex = UINT32_MAX;
  for (uint32_t i = 0; i < deviceCount; i++) {
    if (checkPhysicalDeviceProperties(
            physicalDevices[i],
            &selectedGraphicsQueueFamilyIndex,
            &selectedPresentQueueFamilyIndex)) {
      physicalDevice = physicalDevices[i];
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error(
        "Could not select physical device based on the chosen properties");
  }

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::vector<float> queuePriorities = {1.0f};

  queueCreateInfos.push_back({
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = selectedGraphicsQueueFamilyIndex,
      .queueCount = static_cast<uint32_t>(queuePriorities.size()),
      .pQueuePriorities = queuePriorities.data(),
  });

  if (selectedPresentQueueFamilyIndex != selectedGraphicsQueueFamilyIndex) {
    queueCreateInfos.push_back({
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = selectedPresentQueueFamilyIndex,
        .queueCount = static_cast<uint32_t>(queuePriorities.size()),
        .pQueuePriorities = queuePriorities.data(),
    });
  }

  VkDeviceCreateInfo deviceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
  };

#ifdef NDEBUG
  deviceCreateInfo.enabledLayerCount = 0;
  deviceCreateInfo.ppEnabledLayerNames = nullptr;
#else
  deviceCreateInfo.enabledLayerCount =
      static_cast<uint32_t>(REQUIRED_VALIDATION_LAYERS.size());
  deviceCreateInfo.ppEnabledLayerNames = REQUIRED_VALIDATION_LAYERS.data();
#endif

  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(REQUIRED_DEVICE_EXTENSIONS.size());
  deviceCreateInfo.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data();

  deviceCreateInfo.pEnabledFeatures = nullptr;

  if (vkCreateDevice(
          this->physicalDevice, &deviceCreateInfo, nullptr, &this->device) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create logical device");
  }

  this->graphicsQueueFamilyIndex = selectedGraphicsQueueFamilyIndex;
  this->presentQueueFamilyIndex = selectedPresentQueueFamilyIndex;
}

void VkContext::getDeviceQueues() {
  vkGetDeviceQueue(
      this->device, this->graphicsQueueFamilyIndex, 0, &this->graphicsQueue);
  vkGetDeviceQueue(
      this->device, this->presentQueueFamilyIndex, 0, &this->presentQueue);
}

void VkContext::setupMemoryAllocator() {
  VmaAllocatorCreateInfo allocatorInfo = {
      .physicalDevice = this->physicalDevice,
      .device = this->device,
  };

  vmaCreateAllocator(&allocatorInfo, &this->allocator);
}

void VkContext::createSyncObjects() {
  VkSemaphoreCreateInfo semaphoreCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };

  VkFenceCreateInfo fenceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  for (auto &resources : this->frameResources) {
    if ((vkCreateSemaphore(
             this->device,
             &semaphoreCreateInfo,
             nullptr,
             &resources.imageAvailableSemaphore) != VK_SUCCESS) ||
        (vkCreateSemaphore(
             this->device,
             &semaphoreCreateInfo,
             nullptr,
             &resources.renderingFinishedSemaphore) != VK_SUCCESS) ||
        (vkCreateFence(
             this->device, &fenceCreateInfo, nullptr, &resources.fence) !=
         VK_SUCCESS)) {
      throw std::runtime_error("Failed to create semaphores");
    }
  }
}

void VkContext::createSwapchain(uint32_t width, uint32_t height) {
  for (const auto &imageView : this->swapchainImageViews) {
    if (imageView != VK_NULL_HANDLE) {
      vkDestroyImageView(this->device, imageView, nullptr);
    }
  }
  this->swapchainImageViews.clear();

  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          this->physicalDevice, this->surface, &surfaceCapabilities) !=
      VK_SUCCESS) {
    throw std::runtime_error(
        "Failed to check presentation surface capabilities");
  }

  uint32_t formatCount;
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(
          this->physicalDevice, this->surface, &formatCount, nullptr) !=
          VK_SUCCESS ||
      formatCount == 0) {
    throw std::runtime_error(
        "Error occurred during presentation surface formats enumeration");
  }

  std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(
          this->physicalDevice,
          this->surface,
          &formatCount,
          surfaceFormats.data()) != VK_SUCCESS) {
    throw std::runtime_error(
        "Error occurred during presentation surface formats enumeration");
  }

  uint32_t presentModeCount;
  if (vkGetPhysicalDeviceSurfacePresentModesKHR(
          this->physicalDevice, this->surface, &presentModeCount, nullptr) !=
          VK_SUCCESS ||
      presentModeCount == 0) {
    throw std::runtime_error(
        "Error occurred during presentation surface present modes enumeration");
  }

  std::vector<VkPresentModeKHR> presentModes(presentModeCount);
  if (vkGetPhysicalDeviceSurfacePresentModesKHR(
          this->physicalDevice,
          this->surface,
          &presentModeCount,
          presentModes.data()) != VK_SUCCESS) {
    throw std::runtime_error(
        "Error occurred during presentation surface present modes enumeration");
  }

  uint32_t desiredNumImages = getSwapchainNumImages(surfaceCapabilities);
  VkSurfaceFormatKHR desiredFormat = getSwapchainFormat(surfaceFormats);
  VkExtent2D desiredExtent =
      getSwapchainExtent(width, height, surfaceCapabilities);
  VkImageUsageFlags desiredUsage = getSwapchainUsageFlags(surfaceCapabilities);
  VkSurfaceTransformFlagBitsKHR desiredTransform =
      getSwapchainTransform(surfaceCapabilities);
  VkPresentModeKHR desiredPresentMode = getSwapchainPresentMode(presentModes);

  VkSwapchainKHR oldSwapchain = this->swapchain;

  VkSwapchainCreateInfoKHR createInfo = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .surface = this->surface,
      .minImageCount = desiredNumImages,
      .imageFormat = desiredFormat.format,
      .imageColorSpace = desiredFormat.colorSpace,
      .imageExtent = desiredExtent,
      .imageArrayLayers = 1,
      .imageUsage = desiredUsage,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .preTransform = desiredTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = desiredPresentMode,
      .clipped = VK_TRUE,
      .oldSwapchain = oldSwapchain,
  };

  if (vkCreateSwapchainKHR(
          this->device, &createInfo, nullptr, &this->swapchain) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create swapchain");
  }

  if (oldSwapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(this->device, oldSwapchain, nullptr);
  }

  this->swapchainImageFormat = desiredFormat.format;
  this->swapchainExtent = desiredExtent;

  uint32_t imageCount;
  vkGetSwapchainImagesKHR(device, this->swapchain, &imageCount, nullptr);
  this->swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(
      device, this->swapchain, &imageCount, this->swapchainImages.data());
}

void VkContext::createSwapchainImageViews() {
  this->swapchainImageViews.resize(this->swapchainImages.size());

  for (size_t i = 0; i < swapchainImages.size(); i++) {
    VkImageViewCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = this->swapchainImages[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = this->swapchainImageFormat,
        .components =
            {
                VK_COMPONENT_SWIZZLE_IDENTITY, // r
                VK_COMPONENT_SWIZZLE_IDENTITY, // g
                VK_COMPONENT_SWIZZLE_IDENTITY, // b
                VK_COMPONENT_SWIZZLE_IDENTITY, // a
            },
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    if (vkCreateImageView(
            this->device,
            &createInfo,
            nullptr,
            &this->swapchainImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create image views for framebuffer");
    }
  }
}

void VkContext::createGraphicsCommandPool() {
  VkCommandPoolCreateInfo cmdPoolCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = this->graphicsQueueFamilyIndex,
  };

  if (vkCreateCommandPool(
          this->device,
          &cmdPoolCreateInfo,
          nullptr,
          &this->graphicsCommandPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create graphics command pool");
  }
}

void VkContext::createTransientCommandPool() {
  VkCommandPoolCreateInfo cmdPoolCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
      .queueFamilyIndex = this->graphicsQueueFamilyIndex,
  };

  if (vkCreateCommandPool(
          this->device,
          &cmdPoolCreateInfo,
          nullptr,
          &this->transientCommandPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create transient command pool");
  }
}

void VkContext::allocateGraphicsCommandBuffers() {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkCommandBufferAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = this->graphicsCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    if (vkAllocateCommandBuffers(
            this->device,
            &allocateInfo,
            &this->frameResources[i].commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate command buffers");
    }
  }
}

void VkContext::createDepthResources() {
  for (auto &resources : this->frameResources) {
    this->depthImageFormat = VK_FORMAT_D16_UNORM; // TODO: change this?
    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = this->depthImageFormat,
        .extent =
            {
                .width = this->swapchainExtent.width,
                .height = this->swapchainExtent.height,
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                 VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(
            this->allocator,
            &imageCreateInfo,
            &allocInfo,
            &resources.depthImage,
            &resources.depthImageAllocation,
            nullptr) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create one of the depth images");
    }

    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = resources.depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = this->depthImageFormat,
        .components =
            {
                VK_COMPONENT_SWIZZLE_IDENTITY, // r
                VK_COMPONENT_SWIZZLE_IDENTITY, // g
                VK_COMPONENT_SWIZZLE_IDENTITY, // b
                VK_COMPONENT_SWIZZLE_IDENTITY, // a
            },
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    if (vkCreateImageView(
            this->device,
            &imageViewCreateInfo,
            nullptr,
            &resources.depthImageView) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create one of the depth image views");
    }
  }
}

void VkContext::createRenderPass() {
  std::array<VkAttachmentDescription, 2> attachmentDescriptions{
      VkAttachmentDescription{
          .flags = 0,
          .format = this->swapchainImageFormat,
          .samples = VK_SAMPLE_COUNT_1_BIT,
          .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
          .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
          .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
          .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
          .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // TODO: maybe change
                                                          // this
      },
      VkAttachmentDescription{
          .flags = 0,
          .format = this->depthImageFormat,
          .samples = VK_SAMPLE_COUNT_1_BIT,
          .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
          .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
          .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
          .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
          .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
      },
  };

  VkAttachmentReference colorAttachmentReferences[] = {
      {
          .attachment = 0,
          .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      },
  };

  VkAttachmentReference depthAttachmentReferences[] = {
      {
          .attachment = 1,
          .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      },
  };

  VkSubpassDescription subpassDescriptions[] = {{
      .flags = 0,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = nullptr,
      .colorAttachmentCount = 1,
      .pColorAttachments = colorAttachmentReferences,
      .pResolveAttachments = nullptr,
      .pDepthStencilAttachment = depthAttachmentReferences,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = nullptr,
  }};

  std::vector<VkSubpassDependency> dependencies = {
      {
          .srcSubpass = 0,
          .dstSubpass = VK_SUBPASS_EXTERNAL,
          .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
          .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
          .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
          .dependencyFlags = 0,
      },
  };

  VkRenderPassCreateInfo renderPassCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size()),
      .pAttachments = attachmentDescriptions.data(),
      .subpassCount = 1,
      .pSubpasses = subpassDescriptions,
      .dependencyCount = static_cast<uint32_t>(dependencies.size()),
      .pDependencies = dependencies.data(),
  };

  if (vkCreateRenderPass(
          this->device, &renderPassCreateInfo, nullptr, &this->renderPass) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create render pass");
  }
}

void VkContext::regenFramebuffer(
    VkFramebuffer &framebuffer,
    VkImageView colorImageView,
    VkImageView depthImageView) {
  if (framebuffer != VK_NULL_HANDLE) {
    vkDestroyFramebuffer(this->device, framebuffer, nullptr);
    framebuffer = VK_NULL_HANDLE;
  }

  std::array<VkImageView, 2> attachments{
      colorImageView,
      depthImageView,
  };

  VkFramebufferCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .renderPass = this->renderPass,
      .attachmentCount = static_cast<uint32_t>(attachments.size()),
      .pAttachments = attachments.data(),
      .width = this->swapchainExtent.width,
      .height = this->swapchainExtent.height,
      .layers = 1,
  };

  if (vkCreateFramebuffer(this->device, &createInfo, nullptr, &framebuffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create framebuffer");
  }
}

void VkContext::destroyResizables() {
  if (this->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->device);

    for (auto &resources : this->frameResources) {
      if (resources.commandBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(
            this->device,
            this->graphicsCommandPool,
            1,
            &resources.commandBuffer);
      }

      if (resources.depthImage != VK_NULL_HANDLE) {
        vkDestroyImageView(this->device, resources.depthImageView, nullptr);
        vmaDestroyImage(
            this->allocator,
            resources.depthImage,
            resources.depthImageAllocation);
        resources.depthImage = VK_NULL_HANDLE;
        resources.depthImageView = VK_NULL_HANDLE;
        resources.depthImageAllocation = VK_NULL_HANDLE;
      }
    }

    if (this->renderPass != VK_NULL_HANDLE) {
      vkDestroyRenderPass(this->device, this->renderPass, nullptr);
      this->renderPass = VK_NULL_HANDLE;
    }
  }
}

void VkContext::onResize(uint32_t width, uint32_t height) {
  if (this->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->device);
  }

  this->destroyResizables();

  this->createSwapchain(this->window->getWidth(), this->window->getHeight());
  this->createSwapchainImageViews();
  this->createDepthResources();
  this->createRenderPass();
  this->allocateGraphicsCommandBuffers();
}

void VkContext::present(DrawFunction drawFunction) {
  if (vkWaitForFences(
          this->device,
          1,
          &this->frameResources[this->currentFrame].fence,
          VK_TRUE,
          UINT64_MAX) != VK_SUCCESS) {
    throw std::runtime_error("Waiting for fence took too long");
  }

  vkResetFences(
      this->device, 1, &this->frameResources[this->currentFrame].fence);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      this->device,
      this->swapchain,
      UINT64_MAX,
      this->frameResources[this->currentFrame].imageAvailableSemaphore,
      VK_NULL_HANDLE,
      &imageIndex);

  switch (result) {
  case VK_SUCCESS:
  case VK_SUBOPTIMAL_KHR:
    break;
  case VK_ERROR_OUT_OF_DATE_KHR:
    this->onResize(this->window->getWidth(), this->window->getHeight());
    return;
  default:
    throw std::runtime_error(
        "Problem occurred during swap chain image acquisition");
  }

  VkImageSubresourceRange imageSubresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  };

  {
    this->regenFramebuffer(
        this->frameResources[this->currentFrame].framebuffer,
        this->swapchainImageViews[imageIndex],
        this->frameResources[this->currentFrame].depthImageView);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pInheritanceInfo = nullptr,
    };

    vkBeginCommandBuffer(
        this->frameResources[this->currentFrame].commandBuffer, &beginInfo);

    if (this->presentQueue != this->graphicsQueue) {
      VkImageMemoryBarrier barrierFromPresentToDraw = {
          .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          .pNext = nullptr,
          .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
          .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
          .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
          .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
          .srcQueueFamilyIndex = this->presentQueueFamilyIndex,
          .dstQueueFamilyIndex = this->graphicsQueueFamilyIndex,
          .image = this->swapchainImages[imageIndex],
          .subresourceRange = imageSubresourceRange,
      };

      vkCmdPipelineBarrier(
          this->frameResources[this->currentFrame].commandBuffer,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          0,
          0,
          nullptr,
          0,
          nullptr,
          1,
          &barrierFromPresentToDraw);
    }

    std::array<VkClearValue, 2> clearValues{
        VkClearValue{{{1.0f, 0.8f, 0.4f, 0.0f}}},
        VkClearValue{{{1.0f, 0.0f}}},
    };

    VkRenderPassBeginInfo renderPassBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = this->renderPass,
        .framebuffer = this->frameResources[this->currentFrame].framebuffer,
        .renderArea = {{.x = 0, .y = 0}, this->swapchainExtent},
        .clearValueCount = static_cast<uint32_t>(clearValues.size()),
        .pClearValues = clearValues.data(),
    };

    vkCmdBeginRenderPass(
        this->frameResources[this->currentFrame].commandBuffer,
        &renderPassBeginInfo,
        VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(this->swapchainExtent.width),
        .height = static_cast<float>(this->swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor{{
                         .x = 0,
                         .y = 0,
                     },
                     this->swapchainExtent};

    vkCmdSetViewport(
        this->frameResources[this->currentFrame].commandBuffer,
        0,
        1,
        &viewport);
    vkCmdSetScissor(
        this->frameResources[this->currentFrame].commandBuffer, 0, 1, &scissor);
  }

  // Callback
  drawFunction(this->frameResources[this->currentFrame].commandBuffer);

  {
    vkCmdEndRenderPass(this->frameResources[this->currentFrame].commandBuffer);

    if (this->presentQueue != this->graphicsQueue) {
      VkImageMemoryBarrier barrierFromDrawToPresent = {
          .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          .pNext = nullptr,
          .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
          .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
          .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
          .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
          .srcQueueFamilyIndex = this->graphicsQueueFamilyIndex,
          .dstQueueFamilyIndex = this->presentQueueFamilyIndex,
          .image = this->swapchainImages[imageIndex],
          .subresourceRange = imageSubresourceRange,
      };

      vkCmdPipelineBarrier(
          this->frameResources[this->currentFrame].commandBuffer,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
          0,
          0,
          nullptr,
          0,
          nullptr,
          1,
          &barrierFromDrawToPresent);
    }

    if (vkEndCommandBuffer(
            this->frameResources[this->currentFrame].commandBuffer) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to record command buffers");
    }
  }

  VkPipelineStageFlags waitDstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &this->frameResources[this->currentFrame].imageAvailableSemaphore,
      .pWaitDstStageMask = &waitDstStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers =
          &this->frameResources[this->currentFrame].commandBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores =
          &this->frameResources[this->currentFrame].renderingFinishedSemaphore,
  };

  if (vkQueueSubmit(
          this->graphicsQueue,
          1,
          &submitInfo,
          this->frameResources[this->currentFrame].fence) != VK_SUCCESS) {
    throw std::runtime_error(
        "Failed to submit to the command buffer to presentation queue");
  }

  VkPresentInfoKHR presentInfo = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &this->frameResources[this->currentFrame].renderingFinishedSemaphore,
      .swapchainCount = 1,
      .pSwapchains = &this->swapchain,
      .pImageIndices = &imageIndex,
      .pResults = nullptr,
  };

  result = vkQueuePresentKHR(this->presentQueue, &presentInfo);

  switch (result) {
  case VK_SUCCESS:
    break;
  case VK_ERROR_OUT_OF_DATE_KHR:
  case VK_SUBOPTIMAL_KHR:
    this->onResize(this->window->getWidth(), this->window->getHeight());
    return;
  default:
    throw std::runtime_error("Failed to queue image presentation");
  }

  this->currentFrame = (this->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkResult CreateDebugReportCallbackEXT(
    VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugReportCallbackEXT *pCallback) {
  auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugReportCallbackEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pCallback);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugReportCallbackEXT(
    VkInstance instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugReportCallbackEXT");
  if (func != nullptr) {
    func(instance, callback, pAllocator);
  }
}
