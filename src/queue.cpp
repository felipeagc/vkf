#include "app.hpp"

using namespace app;

void App::getDeviceQueues() {
  vkGetDeviceQueue(
      this->device, this->graphicsQueueFamilyIndex, 0, &this->graphicsQueue);
  vkGetDeviceQueue(
      this->device, this->presentQueueFamilyIndex, 0, &this->presentQueue);
}
