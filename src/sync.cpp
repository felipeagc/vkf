#include "app.hpp"
#include <iostream>

using namespace app;

void App::createSemaphores() {
  VkSemaphoreCreateInfo semaphoreCreateInfo = {};
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreCreateInfo.pNext = nullptr;
  semaphoreCreateInfo.flags = 0;

  if (vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr,
                        &this->imageAvailableSemaphore) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create semaphores");
  }

  if (vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr,
                        &this->renderingFinishedSemaphore) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create semaphores");
  }
}
