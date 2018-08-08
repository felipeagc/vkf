#include "mesh.hpp"
#include "../framework/framework.hpp"
#include <stb_image.h>

using namespace vkf;

Mesh::Mesh(
    StandardMaterial *material,
    std::vector<StandardVertex> vertices,
    std::vector<uint32_t> indices,
    const char *texturePath)
    : framework(material->framework),
      material(material),
      vertices(vertices),
      vertexBuffer(framework, vertices.size() * sizeof(StandardVertex)),
      indices(indices),
      indexBuffer(framework, indices.size() * sizeof(uint32_t)) {
  StagingBuffer *stagingBuffer = this->framework->getStagingBuffer();

  // Vertices
  {
    stagingBuffer->copyMemory(
        vertices.data(), vertices.size() * sizeof(StandardVertex));

    stagingBuffer->transfer(
        vertexBuffer, vertices.size() * sizeof(StandardVertex));
  }

  // Indices
  {
    stagingBuffer->copyMemory(
        indices.data(), indices.size() * sizeof(uint32_t));

    stagingBuffer->transfer(indexBuffer, indices.size() * sizeof(uint32_t));
  }

  // Texture
  {
    uint32_t width, height;
    std::vector<unsigned char> imageData =
        Texture::loadDataFromFile(texturePath, &width, &height);

    this->texture = {this->framework, width, height};

    stagingBuffer->copyMemory(
        imageData.data(), imageData.size() * sizeof(unsigned char));
    stagingBuffer->transfer(texture);
  }

  // Descriptor set
  {
    // TODO: more elegant way of allocating descriptor sets (queue maybe?)
    this->descriptorSetIndex = this->material->getAvailableDescriptorSet();
    if (descriptorSetIndex == -1) {
      throw std::runtime_error("Failed to find available descriptor set");
    }
    this->material->descriptorSetAvailable[descriptorSetIndex] = false;

    VkDescriptorImageInfo imageInfo = {
        .sampler = this->texture.getSamplerHandle(),
        .imageView = this->texture.getImageViewHandle(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet descriptorWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = this->material->descriptorSets[descriptorSetIndex],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &imageInfo,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr,
    };

    vkUpdateDescriptorSets(
        this->framework->getContext()->device, 1, &descriptorWrite, 0, nullptr);
  }
}

Mesh::~Mesh() {
  texture.destroy();
  indexBuffer.destroy();
  vertexBuffer.destroy();

  this->material->descriptorSetAvailable[descriptorSetIndex] = false;
}

void Mesh::draw(VkCommandBuffer commandBuffer) {
  vkCmdBindDescriptorSets(
      commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      this->material->pipelineLayout,
      0,
      1,
      &this->material->descriptorSets[this->descriptorSetIndex],
      0,
      nullptr);

  VkDeviceSize offset = 0;
  VkBuffer vertexBufferHandle = this->vertexBuffer.getHandle();
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferHandle, &offset);
  vkCmdBindIndexBuffer(
      commandBuffer, this->indexBuffer.getHandle(), 0, VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(commandBuffer, this->indices.size(), 1, 0, 0, 0);
}
