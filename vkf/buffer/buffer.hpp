#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace vkf {
class Framework;

class Buffer {
public:
  Buffer(Framework *framework) : framework(framework){};
  ~Buffer(){};

  VkBuffer getHandle();
  void destroy();

protected:
  Framework *framework{nullptr};

  VkBuffer buffer{VK_NULL_HANDLE};
  VmaAllocation allocation{VK_NULL_HANDLE};
};
} // namespace vkf
