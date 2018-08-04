#include "app.hpp"

using namespace app;

void App::createCommandPool(uint32_t queueFamilyIndex, VkCommandPool *pool) {
  VkCommandPoolCreateInfo cmdPoolCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
               VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
      .queueFamilyIndex = queueFamilyIndex};

  if (vkCreateCommandPool(this->device, &cmdPoolCreateInfo, nullptr, pool) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool");
  }
}

void App::allocateCommandBuffers(VkCommandPool pool,
                                 uint32_t count,
                                 VkCommandBuffer *commandBuffers) {
  VkCommandBufferAllocateInfo allocateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = count,
  };

  if (vkAllocateCommandBuffers(this->device, &allocateInfo, commandBuffers) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers");
  }
}

void App::createCommandBuffers() {
  this->createCommandPool(this->graphicsQueueFamilyIndex,
                          &this->graphicsCommandPool);

  this->allocateCommandBuffers(this->graphicsCommandPool,
                               this->graphicsCommandBuffers.size(),
                               this->graphicsCommandBuffers.data());
}
