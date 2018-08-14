#pragma once

#include "../texture/texture.hpp"
#include "buffer.hpp"
#include "index_buffer.hpp"
#include "uniform_buffer.hpp"
#include "vertex_buffer.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace vkf {
class Framework;

class StagingBuffer : public Buffer {
public:
  StagingBuffer(Framework *framework, size_t size);
  ~StagingBuffer(){};

  // Copies memory into the staging buffer
  void copyMemory(void *data, size_t size);

  // Issues a command to transfer this buffer's memory into a vertex buffer
  void transfer(VertexBuffer &buffer, size_t size);

  // Issues a command to transfer this buffer's memory into an index buffer
  void transfer(IndexBuffer &buffer, size_t size);

  // Issues a command to transfer this buffer's memory into a uniform buffer
  void transfer(UniformBuffer &buffer, size_t size);

  // Issues a command to transfer this buffer's memory into a texture's image
  void transfer(Texture &texture);

private:
  void innerBufferTransfer(
      Buffer &buffer,
      size_t size,
      VkAccessFlags dstAccessMask,
      VkPipelineStageFlags dstStageMask);
};
} // namespace vkf
