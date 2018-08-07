#include "buffer_transfer.hpp"

using namespace vkf;

void vkf::transferImage(
    VulkanBackend &backend,
    VkBuffer fromBuffer,
    VkImage image,
    uint32_t width,
    uint32_t height) {
  VkCommandBufferBeginInfo commandBufferBeginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr,
  };

  VkCommandBuffer commandBuffer = backend.graphicsCommandBuffers[0];

  vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

  VkImageSubresourceRange imageSubresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  };

  VkImageMemoryBarrier imageMemoryBarrierFromUndefinedToTransferDst = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = imageSubresourceRange,
  };

  vkCmdPipelineBarrier(
      commandBuffer,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      0,
      0,
      nullptr,
      0,
      nullptr,
      1,
      &imageMemoryBarrierFromUndefinedToTransferDst);

  VkBufferImageCopy bufferImageCopyInfo = {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .imageOffset =
          {
              .x = 0,
              .y = 0,
              .z = 0,
          },
      .imageExtent =
          {
              .width = width,
              .height = height,
              .depth = 1,
          },
  };

  vkCmdCopyBufferToImage(
      commandBuffer,
      fromBuffer,
      image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &bufferImageCopyInfo);

  VkImageMemoryBarrier imageMemoryBarrierFromTransferToShaderRead = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = imageSubresourceRange,
  };

  vkCmdPipelineBarrier(
      commandBuffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      0,
      0,
      nullptr,
      0,
      nullptr,
      1,
      &imageMemoryBarrierFromTransferToShaderRead);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = 0,
      .pWaitSemaphores = nullptr,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
      .signalSemaphoreCount = 0,
      .pSignalSemaphores = nullptr,
  };

  if (vkQueueSubmit(backend.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to submit command buffer to queue");
  }

  vkQueueWaitIdle(backend.graphicsQueue);
}

void vkf::transferBuffer(
    VulkanBackend &backend,
    VkBuffer fromBuffer,
    VkBuffer toBuffer,
    size_t size) {
  VkCommandBufferBeginInfo commandBufferBeginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr,
  };

  vkBeginCommandBuffer(
      backend.graphicsCommandBuffers[0], &commandBufferBeginInfo);

  VkBufferCopy bufferCopyInfo = {
      .srcOffset = 0,
      .dstOffset = 0,
      .size = size,
  };

  vkCmdCopyBuffer(
      backend.graphicsCommandBuffers[0],
      fromBuffer,
      toBuffer,
      1,
      &bufferCopyInfo);

  VkBufferMemoryBarrier bufferMemoryBarrier = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = toBuffer,
      .offset = 0,
      .size = VK_WHOLE_SIZE,
  };

  vkCmdPipelineBarrier(
      backend.graphicsCommandBuffers[0],
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
      0,
      0,
      nullptr,
      1,
      &bufferMemoryBarrier,
      0,
      nullptr);

  vkEndCommandBuffer(backend.graphicsCommandBuffers[0]);

  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = 0,
      .pWaitSemaphores = nullptr,
      .pWaitDstStageMask = nullptr,
      .commandBufferCount = 1,
      .pCommandBuffers = &backend.graphicsCommandBuffers[0],
      .signalSemaphoreCount = 0,
      .pSignalSemaphores = nullptr,
  };

  if (vkQueueSubmit(backend.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to submit command buffer to queue");
  }

  vkQueueWaitIdle(backend.graphicsQueue);
}
