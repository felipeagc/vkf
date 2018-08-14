#pragma once

#include "buffer.hpp"

namespace vkf {
class UniformBuffer : public Buffer {
public:
  UniformBuffer(Framework *framework, size_t size);
};
} // namespace vkf
