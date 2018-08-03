#include "app.hpp"
#include <chrono>
#include <thread>

using namespace app;

App::App() {
}

App::~App() {
  if (this->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->device);

    this->destroyResizables();

    for (const auto &imageView : this->swapchainImageViews) {
      if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(this->device, imageView, nullptr);
      }
    }
    this->swapchainImageViews.clear();

    if (this->swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
    }

    if (this->renderingFinishedSemaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(this->device, this->renderingFinishedSemaphore,
                         nullptr);
    }

    if (this->imageAvailableSemaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(this->device, this->imageAvailableSemaphore, nullptr);
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

void App::run() {
  this->init();

  while (!glfwWindowShouldClose(this->window)) {
    glfwPollEvents();

    if (this->canRender) {
      this->draw();
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
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
  this->createSwapchainImageViews();
  this->createRenderPass();
  this->createFramebuffers();
  this->createPipeline();
  this->createCommandBuffers();
  this->recordCommandBuffers();
}

void App::destroyResizables() {
  if (this->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->device);

    if ((this->graphicsCommandBuffers.size() > 0) &&
        (this->graphicsCommandBuffers[0] != VK_NULL_HANDLE)) {
      vkFreeCommandBuffers(this->device, this->graphicsCommandPool,
                           static_cast<uint32_t>(graphicsCommandBuffers.size()),
                           this->graphicsCommandBuffers.data());
      this->graphicsCommandBuffers.clear();
    }

    if (this->graphicsCommandPool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(this->device, this->graphicsCommandPool, nullptr);
      this->graphicsCommandPool = VK_NULL_HANDLE;
    }

    if (this->graphicsPipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(this->device, this->graphicsPipeline, nullptr);
      this->graphicsPipeline = VK_NULL_HANDLE;
    }

    if (this->renderPass != VK_NULL_HANDLE) {
      vkDestroyRenderPass(this->device, this->renderPass, nullptr);
      this->renderPass = VK_NULL_HANDLE;
    }

    for (const auto &framebuffer : this->framebuffers) {
      if (framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(this->device, framebuffer, nullptr);
      }
    }
    this->framebuffers.clear();
  }
}
