#include "app.hpp"

using namespace app;

void App::destroyCommandPool() {
  if (this->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->device);

    if (this->presentQueueCmdBuffers.size() > 0 &&
        this->presentQueueCmdBuffers[0] != VK_NULL_HANDLE) {
      vkFreeCommandBuffers(this->device, this->presentQueueCmdPool,
                           static_cast<uint32_t>(presentQueueCmdBuffers.size()),
                           this->presentQueueCmdBuffers.data());
      this->presentQueueCmdBuffers.clear();
    }

    if (this->presentQueueCmdPool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(this->device, this->presentQueueCmdPool, nullptr);
      this->presentQueueCmdPool = VK_NULL_HANDLE;
    }
  }
}

void App::createCommandBuffers() {
  VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
  cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmdPoolCreateInfo.pNext = nullptr;
  cmdPoolCreateInfo.flags = 0;
  cmdPoolCreateInfo.queueFamilyIndex = presentQueueFamilyIndex;

  if (vkCreateCommandPool(this->device, &cmdPoolCreateInfo, nullptr,
                          &this->presentQueueCmdPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create a command pool");
  }

  uint32_t imageCount = 0;
  if (vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount,
                              nullptr) != VK_SUCCESS ||
      imageCount == 0) {
    throw std::runtime_error("Failed to get the number of swapchain images");
  }

  this->presentQueueCmdBuffers.resize(imageCount);

  VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {};
  cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdBufferAllocateInfo.pNext = nullptr;
  cmdBufferAllocateInfo.commandPool = this->presentQueueCmdPool;
  cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmdBufferAllocateInfo.commandBufferCount = imageCount;

  if (vkAllocateCommandBuffers(this->device, &cmdBufferAllocateInfo,
                               this->presentQueueCmdBuffers.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers");
  }

  recordCommandBuffers();
}

void App::recordCommandBuffers() {
  uint32_t imageCount =
      static_cast<uint32_t>(this->presentQueueCmdBuffers.size());

  std::vector<VkImage> swapchainImages(imageCount);
  if (vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount,
                              swapchainImages.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to get swapchain images");
  }

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = nullptr;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  beginInfo.pInheritanceInfo = nullptr;

  VkClearColorValue clearColor = {{1.0f, 0.8f, 0.4f, 0.0f}};

  VkImageSubresourceRange imageSubresourceRange = {};
  imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageSubresourceRange.baseMipLevel = 0;
  imageSubresourceRange.levelCount = 1;
  imageSubresourceRange.baseArrayLayer = 0;
  imageSubresourceRange.layerCount = 1;

  for (uint32_t i = 0; i < imageCount; i++) {
    VkImageMemoryBarrier barrierFromPresentToClear = {};
    barrierFromPresentToClear.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierFromPresentToClear.pNext = nullptr;
    barrierFromPresentToClear.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrierFromPresentToClear.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrierFromPresentToClear.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrierFromPresentToClear.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierFromPresentToClear.srcQueueFamilyIndex =
        this->presentQueueFamilyIndex;
    barrierFromPresentToClear.dstQueueFamilyIndex =
        this->presentQueueFamilyIndex;
    barrierFromPresentToClear.image = swapchainImages[i];
    barrierFromPresentToClear.subresourceRange = imageSubresourceRange;

    VkImageMemoryBarrier barrierFromClearToPresent = {};
    barrierFromClearToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierFromClearToPresent.pNext = nullptr;
    barrierFromClearToPresent.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrierFromClearToPresent.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrierFromClearToPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierFromClearToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrierFromClearToPresent.srcQueueFamilyIndex =
        this->presentQueueFamilyIndex;
    barrierFromClearToPresent.dstQueueFamilyIndex =
        this->presentQueueFamilyIndex;
    barrierFromClearToPresent.image = swapchainImages[i];
    barrierFromClearToPresent.subresourceRange = imageSubresourceRange;

    vkBeginCommandBuffer(this->presentQueueCmdBuffers[i], &beginInfo);

    // Convert image to the "transfer dst" layout for the clear operation
    vkCmdPipelineBarrier(this->presentQueueCmdBuffers[i],
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrierFromPresentToClear);

    // Clear the image
    vkCmdClearColorImage(this->presentQueueCmdBuffers[i], swapchainImages[i],
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1,
                         &imageSubresourceRange);

    // Convert the image layout back to the "present source" layout
    vkCmdPipelineBarrier(this->presentQueueCmdBuffers[i],
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrierFromClearToPresent);

    if (vkEndCommandBuffer(this->presentQueueCmdBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to record command buffers");
    }
  }
}
