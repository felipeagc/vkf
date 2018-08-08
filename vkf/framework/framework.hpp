#pragma once

#include "../buffer/staging_buffer.hpp"
#include "../renderer/vk_context.hpp"
#include "../window/window.hpp"

namespace vkf {
const size_t STAGING_BUFFER_SIZE = 1000 * 1000 * 100; // 100 MB

class Framework {
public:
  Framework(const char *title, int width, int height);
  ~Framework();

  Window *getWindow();
  VkContext *getContext();
  StagingBuffer *getStagingBuffer();

protected:
  Window window;
  VkContext context;
  StagingBuffer stagingBuffer{this, STAGING_BUFFER_SIZE};
};
} // namespace vkf
