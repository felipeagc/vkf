#include "app.hpp"

using namespace app;

VkDeviceMemory
App::allocateBufferMemory(VkBuffer &buffer, VkMemoryPropertyFlags properties) {
  VkMemoryRequirements bufferMemoryRequirements;
  vkGetBufferMemoryRequirements(
      this->device, buffer, &bufferMemoryRequirements);

  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memoryProperties);

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((bufferMemoryRequirements.memoryTypeBits & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & properties)) {
      VkMemoryAllocateInfo memoryAllocateInfo = {
          .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
          .pNext = nullptr,
          .allocationSize = bufferMemoryRequirements.size,
          .memoryTypeIndex = i};

      VkDeviceMemory memory;
      if (vkAllocateMemory(
              this->device, &memoryAllocateInfo, nullptr, &memory) !=
          VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate memory");
      }

      return memory;
    }
  }

  throw std::runtime_error("Failed to allocate memory");
}

VkBuffer App::createBuffer(size_t size, VkBufferUsageFlags usage) {
  VkBufferCreateInfo bufferCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr};

  VkBuffer buffer = VK_NULL_HANDLE;
  if (vkCreateBuffer(this->device, &bufferCreateInfo, nullptr, &buffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to vertex buffer");
  }

  return buffer;
}

void App::createStagingBuffer() {
  // TODO: change this later?
  size_t size = 4000;

  this->stagingBuffer =
      this->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

  // The COHERENT property makes sure the mapped memory is always updated and
  // doesn't need explicit flushing
  this->stagingMemory = this->allocateBufferMemory(
      this->stagingBuffer,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (vkBindBufferMemory(
          this->device, this->stagingBuffer, this->stagingMemory, 0) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to bind buffer memory");
  }
}

void App::createVertexBuffer(const std::vector<VertexData> &vertices) {
  size_t size = sizeof(VertexData) * vertices.size();

  this->vertexBuffer = this->createBuffer(
      size,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

  this->vertexMemory = this->allocateBufferMemory(
      this->vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (vkBindBufferMemory(
          this->device, this->vertexBuffer, this->vertexMemory, 0) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to bind buffer memory");
  }
}

void App::copyVertexData(const std::vector<VertexData> &vertices) {
  size_t size = sizeof(VertexData) * vertices.size();

  void *stagingBufferMemoryPointer;
  if (vkMapMemory(
          this->device,
          this->stagingMemory,
          0,
          size,
          0,
          &stagingBufferMemoryPointer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to map memory");
  }

  memcpy(stagingBufferMemoryPointer, vertices.data(), size);

  vkUnmapMemory(this->device, this->stagingMemory);

  VkCommandBufferBeginInfo commandBufferBeginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr};

  VkCommandBuffer commandBuffer = this->graphicsCommandBuffers[0];

  vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

  VkBufferCopy bufferCopyInfo = {
      .srcOffset = 0,
      .dstOffset = 0,
      .size = size,
  };

  vkCmdCopyBuffer(
      commandBuffer,
      this->stagingBuffer,
      this->vertexBuffer,
      1,
      &bufferCopyInfo);

  VkBufferMemoryBarrier bufferMemoryBarrier = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = this->vertexBuffer,
      .offset = 0,
      .size = VK_WHOLE_SIZE};

  vkCmdPipelineBarrier(
      commandBuffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
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

  if (vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to submit command buffer to queue");
  }

  vkQueueWaitIdle(this->graphicsQueue);
}
