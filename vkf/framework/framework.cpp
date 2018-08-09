#include "framework.hpp"

using namespace vkf;

Framework::Framework(const char *title, int width, int height)
    : window(title, width, height), context(&window) {
}

Framework::~Framework() {
  this->stagingBuffer.destroy();
}

Window *Framework::getWindow() {
  return &this->window;
}

VkContext *Framework::getContext() {
  return &this->context;
}

StagingBuffer *Framework::getStagingBuffer() {
  return &this->stagingBuffer;
}
