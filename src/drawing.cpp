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
  this->createPipeline();
  this->createCommandBuffers();
}

void App::prepareFrame(int currentFrame, uint32_t imageIndex) {
  this->regenFramebuffer(
      this->framebuffers[currentFrame], this->swapchainImageViews[imageIndex]);

  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
      .pInheritanceInfo = nullptr,
  };

  vkBeginCommandBuffer(this->graphicsCommandBuffers[currentFrame], &beginInfo);

  VkImageSubresourceRange imageSubresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  };

  if (this->presentQueue != this->graphicsQueue) {
    VkImageMemoryBarrier barrierFromPresentToDraw = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = presentQueueFamilyIndex,
        .dstQueueFamilyIndex = graphicsQueueFamilyIndex,
        .image = swapchainImages[imageIndex],
        .subresourceRange = imageSubresourceRange,
    };

    vkCmdPipelineBarrier(
        this->graphicsCommandBuffers[currentFrame],
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrierFromPresentToDraw);
  }

  VkClearValue clearColor{{{1.0f, 0.8f, 0.4f, 0.0f}}};

  VkRenderPassBeginInfo renderPassBeginInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext = nullptr,
      .renderPass = this->renderPass,
      .framebuffer = this->framebuffers[currentFrame],
      .renderArea = {{.x = 0, .y = 0}, this->swapchainExtent},
      .clearValueCount = 1,
      .pClearValues = &clearColor,
  };

  vkCmdBeginRenderPass(
      this->graphicsCommandBuffers[currentFrame],
      &renderPassBeginInfo,
      VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(
      this->graphicsCommandBuffers[currentFrame],
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      this->graphicsPipeline);

  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(this->swapchainExtent.width),
      .height = static_cast<float>(this->swapchainExtent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };

  VkRect2D scissor{{
                       .x = 0,
                       .y = 0,
                   },
                   this->swapchainExtent};

  vkCmdSetViewport(this->graphicsCommandBuffers[currentFrame], 0, 1, &viewport);
  vkCmdSetScissor(this->graphicsCommandBuffers[currentFrame], 0, 1, &scissor);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(
      this->graphicsCommandBuffers[currentFrame],
      0,
      1,
      &this->vertexBuffer,
      &offset);

  vkCmdBindDescriptorSets(
      this->graphicsCommandBuffers[currentFrame],
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      this->pipelineLayout,
      0,
      1,
      &this->descriptorSet,
      0,
      nullptr);

  vkCmdDraw(this->graphicsCommandBuffers[currentFrame], 4, 1, 0, 0);

  vkCmdEndRenderPass(this->graphicsCommandBuffers[currentFrame]);

  if (this->presentQueue != this->graphicsQueue) {
    VkImageMemoryBarrier barrierFromDrawToPresent = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = graphicsQueueFamilyIndex,
        .dstQueueFamilyIndex = presentQueueFamilyIndex,
        .image = swapchainImages[imageIndex],
        .subresourceRange = imageSubresourceRange,
    };

    vkCmdPipelineBarrier(
        this->graphicsCommandBuffers[currentFrame],
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrierFromDrawToPresent);
  }

  if (vkEndCommandBuffer(this->graphicsCommandBuffers[currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to record command buffers");
  }
}

void App::draw() {
  if (vkWaitForFences(
          this->device,
          1,
          &this->inFlightFences[currentFrame],
          VK_TRUE,
          UINT64_MAX) != VK_SUCCESS) {
    throw std::runtime_error("Waiting for fence took too long");
  }

  vkResetFences(this->device, 1, &this->inFlightFences[currentFrame]);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      this->device,
      this->swapchain,
      UINT64_MAX,
      this->imageAvailableSemaphores[this->currentFrame],
      VK_NULL_HANDLE,
      &imageIndex);

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

  this->prepareFrame(currentFrame, imageIndex);

  VkPipelineStageFlags waitDstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &this->imageAvailableSemaphores[currentFrame],
      .pWaitDstStageMask = &waitDstStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers = &this->graphicsCommandBuffers[currentFrame],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &this->renderingFinishedSemaphores[currentFrame],
  };

  if (vkQueueSubmit(
          this->graphicsQueue,
          1,
          &submitInfo,
          this->inFlightFences[currentFrame]) != VK_SUCCESS) {
    throw std::runtime_error(
        "Failed to submit to the command buffer to presentation queue");
  }

  VkPresentInfoKHR presentInfo = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &this->renderingFinishedSemaphores[currentFrame],
      .swapchainCount = 1,
      .pSwapchains = &this->swapchain,
      .pImageIndices = &imageIndex,
      .pResults = nullptr,
  };

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
