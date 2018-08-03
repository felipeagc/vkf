#include "app.hpp"
#include <iostream>

using namespace app;

void App::onResize() {
  if (this->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->device);
  }

  this->destroyResizables();

  // Recreate stuff
  this->createSwapchain();
  this->createSwapchainImageViews();
  this->createRenderPass();
  this->createFramebuffers();
  this->createPipeline();
  this->createCommandBuffers();
  this->recordCommandBuffers();
}

void App::draw() {
  vkWaitForFences(this->device, 1, &this->inFlightFences[currentFrame], VK_TRUE,
                  UINT64_MAX);
  vkResetFences(this->device, 1, &this->inFlightFences[currentFrame]);

  uint32_t imageIndex;
  VkResult result =
      vkAcquireNextImageKHR(this->device, this->swapchain, UINT64_MAX,
                            this->imageAvailableSemaphores[this->currentFrame],
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
  submitInfo.pWaitSemaphores = &this->imageAvailableSemaphores[currentFrame];
  submitInfo.pWaitDstStageMask = &waitDstStageMask; // ?
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &this->graphicsCommandBuffers[imageIndex];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores =
      &this->renderingFinishedSemaphores[currentFrame];

  if (vkQueueSubmit(this->graphicsQueue, 1, &submitInfo,
                    this->inFlightFences[currentFrame]) != VK_SUCCESS) {
    throw std::runtime_error(
        "Failed to submit to the command buffer to presentation queue");
  }

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores =
      &this->renderingFinishedSemaphores[currentFrame];
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

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
