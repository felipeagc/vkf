#include "app.hpp"

using namespace app;

VkDeviceMemory App::allocateBufferMemory(VkBuffer &buffer) {
  VkMemoryRequirements bufferMemoryRequirements;
  vkGetBufferMemoryRequirements(this->device, buffer,
                                &bufferMemoryRequirements);

  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memoryProperties);

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((bufferMemoryRequirements.memoryTypeBits & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags &
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
      VkMemoryAllocateInfo memoryAllocateInfo = {
          .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
          .pNext = nullptr,
          .allocationSize = bufferMemoryRequirements.size,
          .memoryTypeIndex = i};

      VkDeviceMemory memory;
      if (vkAllocateMemory(this->device, &memoryAllocateInfo, nullptr,
                           &memory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate memory");
      }

      return memory;
    }
  }

  throw std::runtime_error("Failed to allocate memory");
}

VkBuffer App::createBuffer(size_t size) {
  VkBufferCreateInfo bufferCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = size,
      .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
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

void App::createVertexBuffer() {
  std::vector<VertexData> vertices = {
      {{0.0, -0.5, 0.0, 1.0}, {1.0, 0.0, 0.0, 1.0}},
      {{-0.5, 0.5, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}},
      {{0.5, 0.5, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0}},
  };

  size_t size = sizeof(VertexData) * vertices.size();

  this->vertexBuffer = this->createBuffer(size);
  this->memory = this->allocateBufferMemory(this->vertexBuffer);

  if (vkBindBufferMemory(this->device, this->vertexBuffer, memory, 0) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to bind buffer memory");
  }

  void *vertexBufferMemoryPointer;
  if (vkMapMemory(this->device, memory, 0, size, 0,
                  &vertexBufferMemoryPointer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to map memory");
  }

  memcpy(vertexBufferMemoryPointer, vertices.data(), size);

  VkMappedMemoryRange flushRange = {
      .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
      .pNext = nullptr,
      .memory = memory,
      .offset = 0,
      .size = VK_WHOLE_SIZE,
  };

  if (vkFlushMappedMemoryRanges(this->device, 1, &flushRange) != VK_SUCCESS) {
    throw std::runtime_error("Failed to flush mapped memory ranges");
  }

  vkUnmapMemory(this->device, memory);
}
