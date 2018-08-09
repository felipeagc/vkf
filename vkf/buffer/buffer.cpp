#include "buffer.hpp"
#include "../framework/framework.hpp"

using namespace vkf;

VkBuffer Buffer::getHandle() {
  return this->buffer;
}

void Buffer::destroy() {
  if (this->framework->getContext()->getDevice() != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->framework->getContext()->getDevice());

    vmaDestroyBuffer(
        this->framework->getContext()->getAllocator(),
        this->buffer,
        this->allocation);
  }
};
