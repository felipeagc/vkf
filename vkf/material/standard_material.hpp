#pragma once

#include "material.hpp"
#include "../mesh/vertex.hpp"
#include "../renderer/vulkan_backend.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkf {
class StandardMaterial : public Material {
public:
  StandardMaterial(VulkanBackend *backend);
  virtual ~StandardMaterial();

  void bindPipeline(VkCommandBuffer commandBuffer) override;

  void onResize(uint32_t width, uint32_t height) override;

  // Returns the index of the first available descriptor set
  // Returns -1 if not found
  int getAvailableDescriptorSet() override;

protected:
  VkPipelineLayout createPipelineLayout() override;
  void createPipeline() override;
  void createDescriptorSetLayout() override;
  void createDescriptorPool() override;
  void allocateDescriptorSets() override;
};

struct StandardVertex {
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
                .offset = offsetof(StandardVertex, pos),
            },
            {
                .location = 1,
                .binding = binding,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(StandardVertex, color),
            },
            {
                .location = 2,
                .binding = binding,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(StandardVertex, texCoord),
            },
        };

    return vertexAttributeDescriptions;
  }
};
} // namespace vkf
