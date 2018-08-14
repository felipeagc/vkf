#include "mesh.hpp"
#include "../framework/framework.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

using namespace vkf;

Mesh::Mesh(
    StandardMaterial *material,
    std::vector<Vertex> vertices,
    std::vector<uint32_t> indices,
    const char *texturePath)
    : framework(material->framework),
      material(material),
      vertices(vertices),
      vertexBuffer(framework, vertices.size() * sizeof(Vertex)),
      indices(indices),
      indexBuffer(framework, indices.size() * sizeof(uint32_t)),
      uniformBuffer(framework, sizeof(UniformBufferObject)) {
  StagingBuffer *stagingBuffer = this->framework->getStagingBuffer();

  // Vertices
  {
    stagingBuffer->copyMemory(
        vertices.data(), vertices.size() * sizeof(Vertex));

    stagingBuffer->transfer(vertexBuffer, vertices.size() * sizeof(Vertex));
  }

  // Indices
  {
    stagingBuffer->copyMemory(
        indices.data(), indices.size() * sizeof(uint32_t));

    stagingBuffer->transfer(indexBuffer, indices.size() * sizeof(uint32_t));
  }

  // Get a descriptor set
  {
    // TODO: more elegant way of reserving descriptor sets (queue maybe?)
    this->descriptorSetIndex = this->material->getAvailableDescriptorSet();
    if (descriptorSetIndex == -1) {
      throw std::runtime_error("Failed to find available descriptor set");
    }
    this->material->descriptorSetAvailable[descriptorSetIndex] = false;
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

    this->updateTextureDescriptor();
  }
}

Mesh::~Mesh() {
  texture.destroy();
  indexBuffer.destroy();
  vertexBuffer.destroy();
  uniformBuffer.destroy();

  this->material->descriptorSetAvailable[descriptorSetIndex] = true;
}

void Mesh::updateTextureDescriptor() {
  // Update descriptor set
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
      this->framework->getContext()->getDevice(),
      1,
      &descriptorWrite,
      0,
      nullptr);
}

void Mesh::updateUniformDescriptor(UniformBufferObject ubo) {
  // Send UBO data
  StagingBuffer *stagingBuffer = this->framework->getStagingBuffer();

  stagingBuffer->copyMemory(&ubo, sizeof(ubo));
  stagingBuffer->transfer(uniformBuffer, sizeof(ubo));

  // Update descriptor set
  VkDescriptorBufferInfo bufferInfo = {
      .buffer = uniformBuffer.getHandle(),
      .offset = 0,
      .range = sizeof(UniformBufferObject),
  };

  VkWriteDescriptorSet descriptorWrite{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet = this->material->descriptorSets[descriptorSetIndex],
      .dstBinding = 1,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pImageInfo = nullptr,
      .pBufferInfo = &bufferInfo,
      .pTexelBufferView = nullptr,
  };

  vkUpdateDescriptorSets(
      this->framework->getContext()->getDevice(),
      1,
      &descriptorWrite,
      0,
      nullptr);
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
