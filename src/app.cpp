#include "app.hpp"

using namespace app;

App::App() {
}

App::~App() {
}

void App::run() {
  this->init();

  this->cleanup();
}

void App::init() {
  this->createInstance();
#ifndef NDEBUG
  this->setupDebugCallback();
#endif
  this->createDevice();
}

void App::cleanup() {
  if (device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, nullptr);
  }

#ifndef NDEBUG
  if (callback != VK_NULL_HANDLE) {
    DestroyDebugReportCallbackEXT(instance, callback, nullptr);
  }
#endif

  if (instance != VK_NULL_HANDLE) {
    vkDestroyInstance(instance, nullptr);
  }
}
