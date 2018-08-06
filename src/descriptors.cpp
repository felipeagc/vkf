#include "app.hpp"

using namespace app;

void App::createDescriptorSetLayout(VkDescriptorSetLayout *layout) {
  VkDescriptorSetLayoutBinding layoutBinding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .pImmutableSamplers = nullptr,
  };

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = 1,
      .pBindings = &layoutBinding,
  };

  if (vkCreateDescriptorSetLayout(
          this->device, &descriptorSetLayoutCreateInfo, nullptr, layout) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout");
  }
}

void App::createDescriptorPool(VkDescriptorPool *descriptorPool) {
  VkDescriptorPoolSize poolSize = {
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
  };

  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .maxSets = 1,
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize,
  };

  if (vkCreateDescriptorPool(
          this->device, &descriptorPoolCreateInfo, nullptr, descriptorPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool");
  }
}

void App::allocateDescriptorSet(
    VkDescriptorPool descriptorPool,
    VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorSet *descriptorSet) {
  VkDescriptorSetAllocateInfo allocateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = nullptr,
      .descriptorPool = descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptorSetLayout,
  };

  if (vkAllocateDescriptorSets(this->device, &allocateInfo, descriptorSet) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor set");
  }
}

void App::updateDescriptorSetWithTexture(
    VkDescriptorSet descriptorSet, VkSampler sampler, VkImageView imageView) {
  VkDescriptorImageInfo imageInfo = {
      .sampler = sampler,
      .imageView = imageView,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };

  VkWriteDescriptorSet descriptorWrite{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet = descriptorSet,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &imageInfo,
      .pBufferInfo = nullptr,
      .pTexelBufferView = nullptr,
  };

  vkUpdateDescriptorSets(this->device, 1, &descriptorWrite, 0, nullptr);
}
