#include "app.hpp"

using namespace app;

const int WIDTH = 800;
const int HEIGHT = 600;
const char* TITLE = "Vulkan";

void App::createWindow() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  this->window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);
}
