#include "mesh.hpp"
#include "../helper/buffer_transfer.hpp"
#include <stb_image.h>

using namespace vkf;

Mesh::Mesh(
    StandardMaterial *material,
    std::vector<StandardVertex> vertices,
    std::vector<uint32_t> indices)
    : backend(material->backend),
      material(material),
      vertices(vertices),
      indices(indices) {
  int width, height, n_components;
  unsigned char *imageData = stbi_load(
      "../assets/container.jpg",
      &width,
      &height,
      &n_components,
      STBI_rgb_alpha);

  VkDeviceSize imageSize = width * height * 4;

  if (imageData == nullptr) {
    throw std::runtime_error("Failed to load texture file");
  }

  size_t largestSize = imageSize;
  if (largestSize < this->vertices.size() * sizeof(StandardVertex)) {
    largestSize = this->vertices.size() * sizeof(StandardVertex);
  }
  if (largestSize < this->indices.size() * sizeof(uint32_t)) {
    largestSize = this->indices.size() * sizeof(uint32_t);
  }

  VkBuffer stagingBuffer{VK_NULL_HANDLE};
  VmaAllocation stagingAllocation{VK_NULL_HANDLE};

  VkBufferCreateInfo stagingBufferCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = largestSize,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
  };

  VmaAllocationCreateInfo stagingAllocInfo = {};
  stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
  stagingAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  if (vmaCreateBuffer(
          this->backend->allocator,
          &stagingBufferCreateInfo,
          &stagingAllocInfo,
          &stagingBuffer,
          &stagingAllocation,
          nullptr) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create staging buffer");
  }

  // Image

  VkImageCreateInfo imageCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .extent =
          {
              .width = static_cast<uint32_t>(width),
              .height = static_cast<uint32_t>(height),
              .depth = 1,
          },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo imageAllocCreateInfo = {};
  imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  vmaCreateImage(
      this->backend->allocator,
      &imageCreateInfo,
      &imageAllocCreateInfo,
      &this->image,
      &this->imageAllocation,
      nullptr);

  VkImageViewCreateInfo imageViewCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = this->image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .components =
          {
              .r = VK_COMPONENT_SWIZZLE_IDENTITY,
              .g = VK_COMPONENT_SWIZZLE_IDENTITY,
              .b = VK_COMPONENT_SWIZZLE_IDENTITY,
              .a = VK_COMPONENT_SWIZZLE_IDENTITY,
          },
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  if (vkCreateImageView(
          this->backend->device,
          &imageViewCreateInfo,
          nullptr,
          &this->imageView)) {
    throw std::runtime_error("Failed to create image view");
  }

  VkSamplerCreateInfo samplerCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .mipLodBias = 0.0f,
      .anisotropyEnable = VK_FALSE,
      .maxAnisotropy = 1.0f,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .minLod = 0.0f,
      .maxLod = 0.0f,
      .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
  };

  if (vkCreateSampler(
          this->backend->device, &samplerCreateInfo, nullptr, &this->sampler) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create sampler");
  }

  {
    void *stagingMemoryPointer;
    if (vmaMapMemory(
            this->backend->allocator,
            stagingAllocation,
            &stagingMemoryPointer) != VK_SUCCESS) {
      throw std::runtime_error("Failed to map image memory");
    }

    memcpy(stagingMemoryPointer, imageData, imageSize);

    vmaUnmapMemory(this->backend->allocator, stagingAllocation);

    transferImage(
        this->backend,
        stagingBuffer,
        this->image,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height));
  }

  stbi_image_free(imageData);

  // Vertices

  VkBufferCreateInfo vertexBufferCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = this->vertices.size() * sizeof(StandardVertex),
      .usage =
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
  };

  VmaAllocationCreateInfo vertexAllocCreateInfo = {};
  vertexAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  if (vmaCreateBuffer(
          this->backend->allocator,
          &vertexBufferCreateInfo,
          &vertexAllocCreateInfo,
          &this->vertexBuffer,
          &this->vertexAllocation,
          nullptr) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create vertex buffer");
  }

  void *memoryPointer;
  vmaMapMemory(backend->allocator, stagingAllocation, &memoryPointer);
  memcpy(
      memoryPointer, vertices.data(), vertices.size() * sizeof(StandardVertex));
  vmaUnmapMemory(backend->allocator, stagingAllocation);

  transferBuffer(
      this->backend,
      stagingBuffer,
      vertexBuffer,
      vertices.size() * sizeof(StandardVertex));

  // Indices

  VkBufferCreateInfo indexBufferCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = this->indices.size() * sizeof(uint32_t),
      .usage =
          VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
  };

  VmaAllocationCreateInfo indexAllocCreateInfo = {};
  indexAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  if (vmaCreateBuffer(
          this->backend->allocator,
          &indexBufferCreateInfo,
          &indexAllocCreateInfo,
          &this->indexBuffer,
          &this->indexAllocation,
          nullptr) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create index buffer");
  }

  vmaMapMemory(backend->allocator, stagingAllocation, &memoryPointer);
  memcpy(memoryPointer, indices.data(), indices.size() * sizeof(uint32_t));
  vmaUnmapMemory(backend->allocator, stagingAllocation);

  transferBuffer(
      this->backend,
      stagingBuffer,
      indexBuffer,
      indices.size() * sizeof(uint32_t));

  vmaDestroyBuffer(this->backend->allocator, stagingBuffer, stagingAllocation);

  this->descriptorSetIndex = this->material->getAvailableDescriptorSet();
  if (descriptorSetIndex == -1) {
    throw std::runtime_error("Failed to find available descriptor set");
  }
  this->material->descriptorSetAvailable[descriptorSetIndex] = false;

  VkDescriptorImageInfo imageInfo = {
      .sampler = this->sampler,
      .imageView = this->imageView,
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

  vkUpdateDescriptorSets(this->backend->device, 1, &descriptorWrite, 0, nullptr);
}

Mesh::~Mesh() {
  vkQueueWaitIdle(this->backend->graphicsQueue);

  if (this->imageView != VK_NULL_HANDLE) {
    vkDestroyImageView(this->backend->device, this->imageView, nullptr);
    this->imageView = VK_NULL_HANDLE;
  }

  if (this->sampler != VK_NULL_HANDLE) {
    vkDestroySampler(this->backend->device, this->sampler, nullptr);
    this->sampler = VK_NULL_HANDLE;
  }

  vmaDestroyImage(this->backend->allocator, this->image, this->imageAllocation);
  vmaDestroyBuffer(
      this->backend->allocator, this->vertexBuffer, this->vertexAllocation);
  vmaDestroyBuffer(
      this->backend->allocator, this->indexBuffer, this->indexAllocation);

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
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &this->vertexBuffer, &offset);
  vkCmdBindIndexBuffer(
      commandBuffer, this->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(commandBuffer, this->indices.size(), 1, 0, 0, 0);
}
