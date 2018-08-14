#pragma once

#include "../buffer/vertex_buffer.hpp"
#include "../buffer/index_buffer.hpp"
#include "../buffer/uniform_buffer.hpp"
#include "../material/standard_material.hpp"
#include "../texture/texture.hpp"
#include "vertex.hpp"
#include <stb_image.h>
#include <vk_mem_alloc.h>

namespace vkf {
class Framework;

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

class Mesh {
public:
  Mesh(
      StandardMaterial *material,
      std::vector<Vertex> vertices,
      std::vector<uint32_t> indices,
      const char *texturePath);
  ~Mesh();

  void updateTextureDescriptor();
  void updateUniformDescriptor(UniformBufferObject ubo);

  void draw(VkCommandBuffer commandBuffer);

protected:
  Framework *framework;
  StandardMaterial *material;

  int descriptorSetIndex = -1;

  std::vector<Vertex> vertices;
  VertexBuffer vertexBuffer;

  std::vector<uint32_t> indices;
  IndexBuffer indexBuffer;

  UniformBuffer uniformBuffer;

  Texture texture;
};
} // namespace vkf
