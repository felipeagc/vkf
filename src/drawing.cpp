#include "app.hpp"
#include <iostream>

using namespace app;

void App::onResize() {
  if (this->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->device);
  }

  this->destroyCommandPool();

  this->createSwapchain();

  this->createCommandBuffers();
}

void App::draw() {
  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      this->device, this->swapchain, UINT64_MAX, this->imageAvailableSemaphore,
      VK_NULL_HANDLE, &imageIndex);

  switch (result) {
  case VK_SUCCESS:
  case VK_SUBOPTIMAL_KHR:
    break;
  case VK_ERROR_OUT_OF_DATE_KHR:
    onResize();
    return;
  default:
    throw std::runtime_error(
        "Problem occurred during swap chain image acquisition");
  }

  VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = nullptr;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &this->imageAvailableSemaphore;
  submitInfo.pWaitDstStageMask = &waitDstStageMask; // ?
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &this->presentQueueCmdBuffers[imageIndex];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &this->renderingFinishedSemaphore;

  if (vkQueueSubmit(this->presentQueue, 1, &submitInfo, VK_NULL_HANDLE) !=
      VK_SUCCESS) {
    throw std::runtime_error(
        "Failed to submit to the command buffer to presentation queue");
  }

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &this->renderingFinishedSemaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &this->swapchain;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr; // ?

  result = vkQueuePresentKHR(this->presentQueue, &presentInfo);

  switch (result) {
  case VK_SUCCESS:
    break;
  case VK_ERROR_OUT_OF_DATE_KHR:
  case VK_SUBOPTIMAL_KHR:
    onResize();
    return;
  default:
    throw std::runtime_error("Failed to queue image presentation");
  }

  // TODO: temporary
  // Switch this for a "frames-in-flight" approach with VkFences
  vkQueueWaitIdle(this->presentQueue);
}
