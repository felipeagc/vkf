#include "app.hpp"

using namespace app;

void App::createFramebuffers() {
  this->framebuffers.resize(this->swapchainImageViews.size());

  for (size_t i = 0; i < this->swapchainImageViews.size(); i++) {
    VkFramebufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.renderPass = this->renderPass;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &this->swapchainImageViews[i];
    createInfo.width = 300;  // TODO: change this
    createInfo.height = 300; // TODO: change this
    createInfo.layers = 1;

    if (vkCreateFramebuffer(this->device,
                            &createInfo,
                            nullptr,
                            &this->framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer");
    }
  }
}
