#include "app.hpp"

using namespace app;

void App::regenFramebuffer(VkFramebuffer &framebuffer, VkImageView imageView) {
  if (framebuffer != VK_NULL_HANDLE) {
    vkDestroyFramebuffer(this->device, framebuffer, nullptr);
    framebuffer = VK_NULL_HANDLE;
  }

  VkFramebufferCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = 0;
  createInfo.renderPass = this->renderPass;
  createInfo.attachmentCount = 1;
  createInfo.pAttachments = &imageView;
  createInfo.width = this->swapchainExtent.width;
  createInfo.height = this->swapchainExtent.height;
  createInfo.layers = 1;

  if (vkCreateFramebuffer(this->device, &createInfo, nullptr, &framebuffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create framebuffer");
  }
}
