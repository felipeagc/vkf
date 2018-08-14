#include <glm/gtc/matrix_transform.hpp>
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

  vkf::PerspectiveCamera camera{&framework, glm::vec3(0.0, 0.0, -1.0)};

  vkf::Mesh mesh{&material, vertices, indices, "../assets/container.jpg"};

  window->setRelativeMouse(true);

  while (!window->getShouldClose()) {
    window->pollEvents();

    if (window->getRelativeMouse()) {
      const float sensitivity = 0.1f;
      int x, y;
      window->getRelativeMousePos(&x, &y);
      camera.setPitch(camera.getPitch() - glm::radians(y * sensitivity));
      camera.setYaw(camera.getYaw() - glm::radians(x * sensitivity));
    }

    camera.update();

    vkf::UniformBufferObject ubo{
        .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)),
        .view = camera.getViewMatrix(),
        .proj = camera.getProjectionMatrix(),
    };
    mesh.updateUniformDescriptor(ubo);

    context->present([&](VkCommandBuffer commandBuffer) {
      material.bindPipeline(commandBuffer);
      mesh.draw(commandBuffer);
    });
  }

  return 0;
}
