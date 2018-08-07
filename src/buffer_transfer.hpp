#pragma once

#include "vulkan_backend.hpp"

namespace vkf {
void transferImage(
    VulkanBackend &backend,
    VkBuffer fromBuffer,
    VkImage image,
    uint32_t width,
    uint32_t height);
void transferBuffer(
    VulkanBackend &backend,
    VkBuffer fromBuffer,
    VkBuffer toBuffer,
    size_t size);
} // namespace vkf
