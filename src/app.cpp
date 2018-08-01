#include "app.hpp"

using namespace app;

App::App() {
}

App::~App() {
}

void App::run() {
  this->init();

  while (!glfwWindowShouldClose(this->window)) {
    glfwPollEvents();
  }

  this->cleanup();
}

void App::init() {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  this->createWindow();
  this->createInstance();
#ifndef NDEBUG
  this->setupDebugCallback();
#endif
  this->createSurface();
  this->createDevice();
  this->getDeviceQueues();
  this->createSemaphores();
  this->createSwapchain();
}

void App::cleanup() {
  if (this->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->device);

    if (this->imageAvailableSemaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(this->device, this->imageAvailableSemaphore, nullptr);
    }

    if (this->swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
    }

    vkDestroyDevice(this->device, nullptr);
  }

  if (this->surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
  }

#ifndef NDEBUG
  if (this->callback != VK_NULL_HANDLE) {
    DestroyDebugReportCallbackEXT(this->instance, this->callback, nullptr);
  }
#endif

  if (this->instance != VK_NULL_HANDLE) {
    vkDestroyInstance(this->instance, nullptr);
  }

  glfwDestroyWindow(this->window);

  glfwTerminate();
}
