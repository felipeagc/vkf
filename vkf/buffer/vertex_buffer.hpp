#pragma once

#include "buffer.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace vkf {
class Framework;

class VertexBuffer : public Buffer {
public:
  VertexBuffer(Framework *framework, size_t size);
  ~VertexBuffer(){};
};
} // namespace vkf
