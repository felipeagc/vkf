#include "vertex_buffer.hpp"
#include "../framework/framework.hpp"

using namespace vkf;

VertexBuffer::VertexBuffer(Framework *framework, size_t size)
    : Buffer(framework) {
  VkBufferCreateInfo bufferCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = size,
      .usage =
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
  };

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  if (vmaCreateBuffer(
          this->framework->getContext()->getAllocator(),
          &bufferCreateInfo,
          &allocInfo,
          &this->buffer,
          &this->allocation,
          nullptr) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create vertex buffer");
  }
}
