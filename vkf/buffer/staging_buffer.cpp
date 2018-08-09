#include "staging_buffer.hpp"
#include "../framework/framework.hpp"

using namespace vkf;

StagingBuffer::StagingBuffer(Framework *framework, size_t size)
    : Buffer(framework) {
  VkBufferCreateInfo bufferCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = size,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
  };

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
  allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  if (vmaCreateBuffer(
          this->framework->getContext()->getAllocator(),
          &bufferCreateInfo,
          &allocInfo,
          &this->buffer,
          &this->allocation,
          nullptr) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create staging buffer");
  }
}

void StagingBuffer::copyMemory(void *data, size_t size) {
  void *stagingMemoryPointer;
  if (vmaMapMemory(
          this->framework->getContext()->getAllocator(),
          this->allocation,
          &stagingMemoryPointer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to map image memory");
  }

  memcpy(stagingMemoryPointer, data, size);

  vmaUnmapMemory(
      this->framework->getContext()->getAllocator(), this->allocation);
}

void StagingBuffer::transfer(VertexBuffer &buffer, size_t size) {
  this->innerBufferTransfer(
      buffer,
      size,
      VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
}

void StagingBuffer::transfer(IndexBuffer &buffer, size_t size) {
  this->innerBufferTransfer(
      buffer,
      size,
      VK_ACCESS_INDEX_READ_BIT,
      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
}

void StagingBuffer::transfer(Texture &texture) {
  this->framework->getContext()->useTransientCommandBuffer(
      [&](VkCommandBuffer commandBuffer) {
        VkCommandBufferBeginInfo commandBufferBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr,
        };

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
            .image = texture.getImageHandle(),
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
                    .width = texture.getWidth(),
                    .height = texture.getHeight(),
                    .depth = 1,
                },
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            this->buffer,
            texture.getImageHandle(),
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
            .image = texture.getImageHandle(),
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

        if (vkQueueSubmit(
                framework->getContext()->getGraphicsQueue(),
                1,
                &submitInfo,
                VK_NULL_HANDLE) != VK_SUCCESS) {
          throw std::runtime_error("Failed to submit command buffer to queue");
        }

        vkQueueWaitIdle(framework->getContext()->getGraphicsQueue());
      });
}

void StagingBuffer::innerBufferTransfer(
    Buffer &buffer,
    size_t size,
    VkAccessFlags dstAccessMask,
    VkPipelineStageFlags dstStageMask) {
  this->framework->getContext()->useTransientCommandBuffer(
      [&](VkCommandBuffer commandBuffer) {
        VkCommandBufferBeginInfo commandBufferBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr,
        };

        vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

        VkBufferCopy bufferCopyInfo = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        };

        vkCmdCopyBuffer(
            commandBuffer,
            this->buffer,
            buffer.getHandle(),
            1,
            &bufferCopyInfo);

        VkBufferMemoryBarrier bufferMemoryBarrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask = dstAccessMask,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = buffer.getHandle(),
            .offset = 0,
            .size = VK_WHOLE_SIZE,
        };

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            dstStageMask,
            0,
            0,
            nullptr,
            1,
            &bufferMemoryBarrier,
            0,
            nullptr);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
        };

        if (vkQueueSubmit(
                framework->getContext()->getGraphicsQueue(),
                1,
                &submitInfo,
                VK_NULL_HANDLE) != VK_SUCCESS) {
          throw std::runtime_error("Failed to submit command buffer to queue");
        }

        vkQueueWaitIdle(framework->getContext()->getGraphicsQueue());
      });
}
