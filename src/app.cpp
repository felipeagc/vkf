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

    for (const auto &framebuffer : this->framebuffers) {
      if (framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(this->device, framebuffer, nullptr);
      }
    }

    for (const auto &imageView : this->swapchainImageViews) {
      if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(this->device, imageView, nullptr);
      }
    }

    if (this->swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (this->inFlightFences[i] != VK_NULL_HANDLE) {
        vkDestroyFence(this->device, this->inFlightFences[i], nullptr);
      }

      if (this->renderingFinishedSemaphores[i] != VK_NULL_HANDLE) {
        vkDestroySemaphore(
            this->device, this->renderingFinishedSemaphores[i], nullptr);
      }

      if (this->imageAvailableSemaphores[i] != VK_NULL_HANDLE) {
        vkDestroySemaphore(
            this->device, this->imageAvailableSemaphores[i], nullptr);
      }
    }

    if (this->vertexBuffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(this->device, this->vertexBuffer, nullptr);
      this->vertexBuffer = VK_NULL_HANDLE;
    }

    if (this->vertexMemory != VK_NULL_HANDLE) {
      vkFreeMemory(this->device, this->vertexMemory, nullptr);
      this->vertexMemory = VK_NULL_HANDLE;
    }

    if (this->stagingBuffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(this->device, this->stagingBuffer, nullptr);
      this->stagingBuffer = VK_NULL_HANDLE;
    }

    if (this->stagingMemory != VK_NULL_HANDLE) {
      vkFreeMemory(this->device, this->stagingMemory, nullptr);
      this->stagingMemory = VK_NULL_HANDLE;
    }

    if (this->textureImage != VK_NULL_HANDLE) {
      vkDestroyImage(this->device, this->textureImage, nullptr);
      this->textureImage = VK_NULL_HANDLE;
    }

    if (this->textureImageView != VK_NULL_HANDLE) {
      vkDestroyImageView(this->device, this->textureImageView, nullptr);
      this->textureImageView = VK_NULL_HANDLE;
    }

    if (this->textureSampler != VK_NULL_HANDLE) {
      vkDestroySampler(this->device, this->textureSampler, nullptr);
      this->textureSampler = VK_NULL_HANDLE;
    }

    if (this->textureMemory != VK_NULL_HANDLE) {
      vkFreeMemory(this->device, this->textureMemory, nullptr);
      this->textureMemory = VK_NULL_HANDLE;
    }

    if (this->descriptorPool != VK_NULL_HANDLE) {
      vkDestroyDescriptorPool(this->device, this->descriptorPool, nullptr);
      this->descriptorPool = VK_NULL_HANDLE;
    }

    if (this->descriptorSetLayout != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(
          this->device, this->descriptorSetLayout, nullptr);
      this->descriptorSetLayout = VK_NULL_HANDLE;
    }

    if (this->pipelineLayout != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);
      this->pipelineLayout = VK_NULL_HANDLE;
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
  this->createSyncObjects();
  this->createSwapchain();
  this->createSwapchainImageViews();
  this->createRenderPass();
  this->createCommandBuffers();

  this->createDescriptorSetLayout(&this->descriptorSetLayout);

  this->createPipeline();

  this->createDescriptorPool(&this->descriptorPool);
  this->allocateDescriptorSet(
      this->descriptorPool, this->descriptorSetLayout, &this->descriptorSet);

  this->createStagingBuffer();

  std::vector<VertexData> vertices = {
      // top left
      {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      // bottom left
      {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
      // top right
      {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
      // bottom right
      {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
  };

  this->createVertexBuffer(vertices);
  this->copyVertexData(vertices);

  this->createTexture();

  this->updateDescriptorSetWithTexture(
      this->descriptorSet, this->textureSampler, this->textureImageView);
}

void App::destroyResizables() {
  if (this->device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->device);

    if ((this->graphicsCommandBuffers.size() > 0) &&
        (this->graphicsCommandBuffers[0] != VK_NULL_HANDLE)) {
      vkFreeCommandBuffers(
          this->device,
          this->graphicsCommandPool,
          static_cast<uint32_t>(graphicsCommandBuffers.size()),
          this->graphicsCommandBuffers.data());
    }

    if (this->graphicsCommandPool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(this->device, this->graphicsCommandPool, nullptr);
      this->graphicsCommandPool = VK_NULL_HANDLE;
    }

    if (this->pipelineLayout != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);
      this->pipelineLayout = VK_NULL_HANDLE;
    }

    if (this->graphicsPipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(this->device, this->graphicsPipeline, nullptr);
      this->graphicsPipeline = VK_NULL_HANDLE;
    }

    if (this->renderPass != VK_NULL_HANDLE) {
      vkDestroyRenderPass(this->device, this->renderPass, nullptr);
      this->renderPass = VK_NULL_HANDLE;
    }
  }
}
