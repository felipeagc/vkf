#include <iostream>
#include <vkf.hpp>

int main() {
  vkf::Framework framework("vkf", 800, 600);
  auto window = framework.getWindow();
  auto backend = framework.getBackend();

  vkf::StandardMaterial material{framework.getBackend()};

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

  window->setOnResize([&](uint32_t width, uint32_t height) {
    backend->onResize(width, height);
    material.onResize(width, height);
  });

  while (!window->getShouldClose()) {
    window->pollEvents();

    backend->present(
        window->getWidth(),
        window->getHeight(),
        [&](VkCommandBuffer commandBuffer) {
          material.bindPipeline(commandBuffer);
          mesh.draw(commandBuffer);
        });
  }

  return 0;
}
