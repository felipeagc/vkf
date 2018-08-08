#include "texture.hpp"
#include "../framework/framework.hpp"
#include <stb_image.h>

using namespace vkf;

Texture::Texture() {
}

Texture::Texture(Framework *framework, uint32_t width, uint32_t height)
    : framework(framework), width(width), height(height) {
  VkImageCreateInfo imageCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .extent =
          {
              .width = width,
              .height = height,
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
      this->framework->getContext()->allocator,
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
          this->framework->getContext()->device,
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
          this->framework->getContext()->device,
          &samplerCreateInfo,
          nullptr,
          &this->sampler) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create sampler");
  }
}

void Texture::destroy() {
  if (this->framework->getContext()->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->framework->getContext()->device);

    if (this->sampler != VK_NULL_HANDLE) {
      vkDestroySampler(
          this->framework->getContext()->device, this->sampler, nullptr);
      this->sampler = VK_NULL_HANDLE;
    }

    if (this->imageView != VK_NULL_HANDLE) {
      vkDestroyImageView(
          this->framework->getContext()->device, this->imageView, nullptr);
      this->imageView = VK_NULL_HANDLE;
    }

    if (this->image != VK_NULL_HANDLE) {
      vmaDestroyImage(
          this->framework->getContext()->allocator,
          this->image,
          this->imageAllocation);
    }
  }
}

VkImage Texture::getImageHandle() {
  return this->image;
}

VkImageView Texture::getImageViewHandle() {
  return this->imageView;
}

VkSampler Texture::getSamplerHandle() {
  return this->sampler;
}

uint32_t Texture::getWidth() const {
  return this->width;
}

uint32_t Texture::getHeight() const {
  return this->height;
}
