#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkf {
struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static std::vector<VkVertexInputAttributeDescription>
  getVertexAttributeDescriptions(const uint32_t binding) {
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions =
        {
            {
                .location = 0,
                .binding = binding,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, pos),
            },
            {
                .location = 1,
                .binding = binding,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color),
            },
            {
                .location = 2,
                .binding = binding,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, texCoord),
            },
        };

    return vertexAttributeDescriptions;
  }
};
} // namespace vkf
