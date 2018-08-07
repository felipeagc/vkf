#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace vkf {
struct Vertex {
  static std::vector<VkVertexInputAttributeDescription>
  getVertexAttributeDescriptions(const uint32_t binding);
};
} // namespace vkf
