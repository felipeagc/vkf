#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace app {
struct VertexData {
  glm::vec3 pos;
  glm::vec3 color;

  static std::vector<VkVertexInputAttributeDescription>
  getVertexAttributeDescriptions(const uint32_t binding) {
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions =
        {
            {
                .location = 0,
                .binding = binding,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(VertexData, pos),
            },
            {
                .location = 1,
                .binding = binding,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(VertexData, color),
            },
        };

    return vertexAttributeDescriptions;
  }
};
} // namespace app
