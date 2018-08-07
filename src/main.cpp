#include "mesh.hpp"
#include "standard_material.hpp"
#include "vulkan_backend.hpp"
#include <iostream>

int main() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);

  SDL_Window *window = SDL_CreateWindow(
      "vkf",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      800,
      600,
      SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

  try {
    uint32_t sdlExtensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);
    std::vector<const char *> sdlExtensions(sdlExtensionCount);
    SDL_Vulkan_GetInstanceExtensions(
        window, &sdlExtensionCount, sdlExtensions.data());

    vkf::VulkanBackend backend{window, sdlExtensions};
    vkf::StandardMaterial material{backend};

    std::vector<vkf::StandardVertex> vertices = {
        // top left
        {{-0.5, -0.5, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0}},
        // top right
        {{0.5, -0.5, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0}},
        // bottom right
        {{0.5, 0.5, 0.0}, {0.0, 0.0, 1.0}, {1.0, 1.0}},
        // bottom left
        {{-0.5, 0.5, 0.0}, {0.0, 1.0, 1.0}, {0.0, 1.0}},
    };
    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
    vkf::Mesh mesh{&material, vertices, indices};

    bool shouldRun = true;
    SDL_Event event;

    // pipeline.updateDescriptorSet(mesh.sampler, mesh.imageView);

    while (shouldRun) {
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
          shouldRun = false;
          break;
        case SDL_WINDOWEVENT:
          switch (event.window.type) {
          case SDL_WINDOWEVENT_RESIZED:
            backend.onResize(
                static_cast<uint32_t>(event.window.data1),
                static_cast<uint32_t>(event.window.data2));
            material.onResize(
                static_cast<uint32_t>(event.window.data1),
                static_cast<uint32_t>(event.window.data2));
            break;
          }
          break;
        }
      }

      int width, height;
      SDL_GetWindowSize(window, &width, &height);

      backend.present(
          static_cast<uint32_t>(width),
          static_cast<uint32_t>(height),
          [&](VkCommandBuffer commandBuffer) {
            material.bindPipeline(commandBuffer);

            mesh.draw(commandBuffer);

            // VkDeviceSize offset = 0;
            // vkCmdBindVertexBuffers(
            //     backend.graphicsCommandBuffers[currentFrame],
            //     0,
            //     1,
            //     &bavertexBuffer,
            //     &offset);

            // vkCmdDraw(this->graphicsCommandBuffers[currentFrame], 4, 1, 0,
            // 0);
          });
    }
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
