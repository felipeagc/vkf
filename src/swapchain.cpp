#include "app.hpp"
#include <iostream>

using namespace app;

uint32_t App::getSwapchainNumImages(
    const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
  uint32_t imageCount = surfaceCapabilities.minImageCount + 1;

  if (surfaceCapabilities.minImageCount > 0 &&
      imageCount > surfaceCapabilities.maxImageCount) {
    imageCount = surfaceCapabilities.maxImageCount;
  }

  return imageCount;
}

VkSurfaceFormatKHR
App::getSwapchainFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
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

VkExtent2D
App::getSwapchainExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
  if (surfaceCapabilities.currentExtent.width == static_cast<uint32_t>(-1)) {
    int width, height;
    glfwGetWindowSize(this->window, &width, &height);

    VkExtent2D swapchainExtent = {static_cast<uint32_t>(width),
                                  static_cast<uint32_t>(height)};
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

VkImageUsageFlags App::getSwapchainUsageFlags(
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

VkSurfaceTransformFlagBitsKHR App::getSwapchainTransform(
    const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
  if (surfaceCapabilities.supportedTransforms &
      VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
    return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  } else {
    return surfaceCapabilities.currentTransform;
  }
}

VkPresentModeKHR App::getSwapchainPresentMode(
    const std::vector<VkPresentModeKHR> &presentModes) {
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

void App::createSwapchain() {
  canRender = false;

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
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface,
                                           &formatCount,
                                           nullptr) != VK_SUCCESS ||
      formatCount == 0) {
    throw std::runtime_error(
        "Error occurred during presentation surface formats enumeration");
  }

  std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(
          this->physicalDevice, this->surface, &formatCount,
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
          this->physicalDevice, this->surface, &presentModeCount,
          presentModes.data()) != VK_SUCCESS) {
    throw std::runtime_error(
        "Error occurred during presentation surface present modes enumeration");
  }

  uint32_t desiredNumImages = getSwapchainNumImages(surfaceCapabilities);
  VkSurfaceFormatKHR desiredFormat = getSwapchainFormat(surfaceFormats);
  VkExtent2D desiredExtent = getSwapchainExtent(surfaceCapabilities);
  VkImageUsageFlags desiredUsage = getSwapchainUsageFlags(surfaceCapabilities);
  VkSurfaceTransformFlagBitsKHR desiredTransform =
      getSwapchainTransform(surfaceCapabilities);
  VkPresentModeKHR desiredPresentMode = getSwapchainPresentMode(presentModes);

  VkSwapchainKHR oldSwapchain = this->swapchain;

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.pNext = nullptr;
  createInfo.flags = 0;
  createInfo.surface = this->surface;
  createInfo.minImageCount = desiredNumImages;
  createInfo.imageFormat = desiredFormat.format;
  createInfo.imageColorSpace = desiredFormat.colorSpace;
  createInfo.imageExtent = desiredExtent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = desiredUsage;
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.queueFamilyIndexCount = 0;
  createInfo.pQueueFamilyIndices = nullptr;
  createInfo.preTransform = desiredTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = desiredPresentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = oldSwapchain;

  if (vkCreateSwapchainKHR(this->device, &createInfo, nullptr,
                           &this->swapchain) != VK_SUCCESS) {
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
  vkGetSwapchainImagesKHR(device, this->swapchain, &imageCount,
                          this->swapchainImages.data());

  canRender = true;
}

void App::createSwapchainImageViews() {
  this->swapchainImageViews.resize(this->swapchainImages.size());

  for (size_t i = 0; i < swapchainImages.size(); i++) {
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.image = this->swapchainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = this->swapchainImageFormat;
    createInfo.components = {
        VK_COMPONENT_SWIZZLE_IDENTITY, // r
        VK_COMPONENT_SWIZZLE_IDENTITY, // g
        VK_COMPONENT_SWIZZLE_IDENTITY, // b
        VK_COMPONENT_SWIZZLE_IDENTITY, // a
    };
    createInfo.subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    if (vkCreateImageView(this->device, &createInfo, nullptr,
                          &this->swapchainImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create image views for framebuffer");
    }
  }
}
