#pragma once

#include "buffer.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace vkf {
class Framework;

class IndexBuffer : public Buffer {
public:
  IndexBuffer(Framework *framework, size_t size);
  ~IndexBuffer(){};
};
} // namespace vkf
