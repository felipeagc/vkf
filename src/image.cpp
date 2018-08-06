#include "app.hpp"
#include <stb_image.h>

using namespace app;

void App::createImage(uint32_t width, uint32_t height, VkImage *image) {
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

  if (vkCreateImage(this->device, &imageCreateInfo, nullptr, image) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create image");
  }
}

void App::allocateImageMemory(
    VkImage &image, VkMemoryPropertyFlags property, VkDeviceMemory *memory) {
  VkMemoryRequirements imageMemoryRequirements;
  vkGetImageMemoryRequirements(this->device, image, &imageMemoryRequirements);

  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memoryProperties);

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((imageMemoryRequirements.memoryTypeBits & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & property)) {
      VkMemoryAllocateInfo memoryAllocateInfo = {
          .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
          .pNext = nullptr,
          .allocationSize = imageMemoryRequirements.size,
          .memoryTypeIndex = i,
      };

      if (vkAllocateMemory(
              this->device, &memoryAllocateInfo, nullptr, memory)) {
        throw std::runtime_error("Failed to allocate image memory");
      }
    }
  }
}

void App::createImageView(const VkImage image, VkImageView *imageView) {
  VkImageViewCreateInfo imageViewCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = image,
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
          this->device, &imageViewCreateInfo, nullptr, imageView)) {
    throw std::runtime_error("Failed to create image view");
  }
}

void App::createSampler(VkSampler *sampler) {
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

  if (vkCreateSampler(this->device, &samplerCreateInfo, nullptr, sampler) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create sampler");
  }
}

void App::createTexture() {
  int width, height, n_components;
  unsigned char *data = stbi_load(
      "../assets/container.jpg", &width, &height, &n_components, STBI_rgb_alpha);

  VkDeviceSize imageSize = width * height * 4;

  if (data == nullptr) {
    throw std::runtime_error("Failed to load texture file");
  }

  this->createImage(
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height),
      &this->textureImage);

  this->allocateImageMemory(
      this->textureImage,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      &this->textureMemory);

  if (vkBindImageMemory(
          this->device, this->textureImage, this->textureMemory, 0) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to bind memory to an image");
  }

  this->createImageView(this->textureImage, &this->textureImageView);

  this->createSampler(&this->textureSampler);

  this->copyTextureData(
      data,
      imageSize,
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height));

  stbi_image_free(data);

  VkCommandBufferBeginInfo commandBufferBeginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr,
  };

  VkCommandBuffer commandBuffer = this->graphicsCommandBuffers[0];

  vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

  VkImageSubresourceRange imageSubresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  };

  VkImageMemoryBarrier imageMemoryBarrierFromUndefinedToTransferDst = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = this->textureImage,
      .subresourceRange = imageSubresourceRange,
  };

  vkCmdPipelineBarrier(
      commandBuffer,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      0,
      0,
      nullptr,
      0,
      nullptr,
      1,
      &imageMemoryBarrierFromUndefinedToTransferDst);

  VkBufferImageCopy bufferImageCopyInfo = {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .imageOffset =
          {
              .x = 0,
              .y = 0,
              .z = 0,
          },
      .imageExtent =
          {
              .width = static_cast<uint32_t>(width),
              .height = static_cast<uint32_t>(height),
              .depth = 1,
          },
  };

  vkCmdCopyBufferToImage(
      commandBuffer,
      this->stagingBuffer,
      this->textureImage,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &bufferImageCopyInfo);

  VkImageMemoryBarrier imageMemoryBarrierFromTransferToShaderRead = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = this->textureImage,
      .subresourceRange = imageSubresourceRange,
  };

  vkCmdPipelineBarrier(
      commandBuffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      0,
      0,
      nullptr,
      0,
      nullptr,
      1,
      &imageMemoryBarrierFromTransferToShaderRead);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = 0,
      .pWaitSemaphores = nullptr,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
      .signalSemaphoreCount = 0,
      .pSignalSemaphores = nullptr,
  };

  if (vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to submit command buffer to queue");
  }

  vkQueueWaitIdle(this->graphicsQueue);
}

void App::copyTextureData(
    unsigned char *textureData,
    size_t dataSize,
    uint32_t width,
    uint32_t height) {
  void *stagingBufferMemoryPointer;
  if (vkMapMemory(
          this->device,
          this->stagingMemory,
          0,
          dataSize,
          0,
          &stagingBufferMemoryPointer) != VK_SUCCESS) {
  }

  memcpy(stagingBufferMemoryPointer, textureData, dataSize);

  // No need to flush, since the staging buffer is using the coherent property

  vkUnmapMemory(this->device, this->stagingMemory);
}
