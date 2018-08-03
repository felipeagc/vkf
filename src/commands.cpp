#include "app.hpp"

using namespace app;

void App::createCommandBuffers() {
  this->createCommandPool(this->graphicsQueueFamilyIndex,
                          &this->graphicsCommandPool);

  this->graphicsCommandBuffers.resize(this->swapchainImages.size());

  this->allocateCommandBuffers(this->graphicsCommandPool,
                               this->graphicsCommandBuffers.size(),
                               this->graphicsCommandBuffers.data());
}

void App::recordCommandBuffers() {
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = nullptr;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  beginInfo.pInheritanceInfo = nullptr;

  VkImageSubresourceRange imageSubresourceRange = {};
  imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageSubresourceRange.baseMipLevel = 0;
  imageSubresourceRange.levelCount = 1;
  imageSubresourceRange.baseArrayLayer = 0;
  imageSubresourceRange.layerCount = 1;

  VkClearValue clearColor{{{1.0f, 0.8f, 0.4f, 0.0f}}};

  for (uint32_t i = 0; i < this->graphicsCommandBuffers.size(); i++) {
    vkBeginCommandBuffer(this->graphicsCommandBuffers[i], &beginInfo);

    if (this->presentQueue != this->graphicsQueue) {
      VkImageMemoryBarrier barrierFromPresentToDraw = {
          .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          .pNext = nullptr,
          .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
          .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
          .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
          .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
          .srcQueueFamilyIndex = this->presentQueueFamilyIndex,
          .dstQueueFamilyIndex = this->graphicsQueueFamilyIndex,
          .image = swapchainImages[i],
          .subresourceRange = imageSubresourceRange};

      // Convert image to the "transfer dst" layout for the clear operation
      vkCmdPipelineBarrier(this->graphicsCommandBuffers[i],
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrierFromPresentToDraw);
    }

    VkRenderPassBeginInfo renderPassBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = this->renderPass,
        .framebuffer = this->framebuffers[i],
        .renderArea = {{.x = 0, .y = 0}, {.width = 300, .height = 300}},
        .clearValueCount = 1,
        .pClearValues = &clearColor};

    vkCmdBeginRenderPass(this->graphicsCommandBuffers[i], &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(this->graphicsCommandBuffers[i],
                      VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);

    vkCmdDraw(this->graphicsCommandBuffers[i], 3, 1, 0, 0);

    vkCmdEndRenderPass(this->graphicsCommandBuffers[i]);

    if (this->presentQueue != this->graphicsQueue) {
      VkImageMemoryBarrier barrierFromDrawToPresent = {
          .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          .pNext = nullptr,
          .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
          .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
          .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
          .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
          .srcQueueFamilyIndex = this->graphicsQueueFamilyIndex,
          .dstQueueFamilyIndex = this->presentQueueFamilyIndex,
          .image = swapchainImages[i],
          .subresourceRange = imageSubresourceRange,
      };

      // Convert the image layout back to the "present source" layout
      vkCmdPipelineBarrier(this->graphicsCommandBuffers[i],
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &barrierFromDrawToPresent);
    }

    if (vkEndCommandBuffer(this->graphicsCommandBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to record command buffers");
    }
  }
}

void App::createCommandPool(uint32_t queueFamilyIndex, VkCommandPool *pool) {
  VkCommandPoolCreateInfo cmdPoolCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = queueFamilyIndex};

  if (vkCreateCommandPool(this->device, &cmdPoolCreateInfo, nullptr, pool) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool");
  }
}

void App::allocateCommandBuffers(VkCommandPool pool,
                                 uint32_t count,
                                 VkCommandBuffer *commandBuffers) {
  VkCommandBufferAllocateInfo allocateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = count,
  };

  if (vkAllocateCommandBuffers(this->device, &allocateInfo, commandBuffers) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers");
  }
}
