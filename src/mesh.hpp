#pragma once

#include "standard_material.hpp"
#include "vertex.hpp"
#include "vulkan_backend.hpp"
#include <stb_image.h>

namespace vkf {
class Mesh {
public:
  Mesh(
      StandardMaterial *material,
      std::vector<StandardVertex> vertices,
      std::vector<uint32_t> indices);

  virtual ~Mesh();

  void draw(VkCommandBuffer commandBuffer);

protected:
  VulkanBackend &backend;
  StandardMaterial *material;

  int descriptorSetIndex = -1;

  std::vector<StandardVertex> vertices;
  VkBuffer vertexBuffer{VK_NULL_HANDLE};
  VmaAllocation vertexAllocation{VK_NULL_HANDLE};

  std::vector<uint32_t> indices;
  VkBuffer indexBuffer{VK_NULL_HANDLE};
  VmaAllocation indexAllocation{VK_NULL_HANDLE};

  VkImage image{VK_NULL_HANDLE};
  VkImageView imageView{VK_NULL_HANDLE};
  VkSampler sampler{VK_NULL_HANDLE};
  VmaAllocation imageAllocation{VK_NULL_HANDLE};
};
} // namespace vkf
