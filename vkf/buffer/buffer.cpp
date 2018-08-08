#include "buffer.hpp"
#include "../framework/framework.hpp"

using namespace vkf;

VkBuffer Buffer::getHandle() {
  return this->buffer;
}

void Buffer::destroy() {
  if (this->framework->getContext()->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->framework->getContext()->device);

    vmaDestroyBuffer(
        this->framework->getContext()->allocator,
        this->buffer,
        this->allocation);
  }
};
