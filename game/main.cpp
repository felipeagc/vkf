#include <iostream>
#include <vkf.hpp>

int main() {
  vkf::Framework framework("vkf", 800, 600);
  auto window = framework.getWindow();
  auto context = framework.getContext();

  vkf::StandardMaterial material{&framework};

  std::vector<vkf::Vertex> vertices = {
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
  vkf::Mesh mesh{&material, vertices, indices, "../assets/container.jpg"};

  while (!window->getShouldClose()) {
    window->pollEvents();

    context->present([&](VkCommandBuffer commandBuffer) {
      material.bindPipeline(commandBuffer);
      mesh.draw(commandBuffer);
    });
  }

  return 0;
}
