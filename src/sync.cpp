#include "app.hpp"
#include <iostream>

using namespace app;

void App::createSyncObjects() {
  VkSemaphoreCreateInfo semaphoreCreateInfo = {};
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreCreateInfo.pNext = nullptr;
  semaphoreCreateInfo.flags = 0;

  VkFenceCreateInfo fenceCreateInfo = {};
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.pNext = nullptr;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if ((vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr,
                           &this->imageAvailableSemaphores[i]) != VK_SUCCESS) ||
        (vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr,
                           &this->renderingFinishedSemaphores[i]) !=
         VK_SUCCESS) ||
        (vkCreateFence(this->device, &fenceCreateInfo, nullptr,
                       &inFlightFences[i]))) {
      throw std::runtime_error("Failed to create semaphores");
    }
  }
}
