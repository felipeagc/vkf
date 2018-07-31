#include "app.hpp"
#include <iostream>

using namespace app;

bool checkPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                   uint32_t *queueFamilyIndex) {
  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceFeatures deviceFeatures;

  vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
  vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

  uint32_t majorVersion = VK_VERSION_MAJOR(deviceProperties.apiVersion);
  // uint32_t minorVersion = VK_VERSION_MINOR(deviceProperties.apiVersion);
  // uint32_t patchVersion = VK_VERSION_PATCH(deviceProperties.apiVersion);

  if (majorVersion < 1 && deviceProperties.limits.maxImageDimension2D < 4096) {
    std::cout << "Physical device " << physicalDevice
              << " doesn't support required parameters!" << std::endl;
    return false;
  }

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           nullptr);

  if (queueFamilyCount == 0) {
    std::cout << "Physical device " << physicalDevice
              << " doesn't have any queue families" << std::endl;
    return false;
  }

  std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           queueFamilyProperties.data());

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    if (queueFamilyProperties[i].queueCount > 0 &&
        queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      *queueFamilyIndex = i;
      return true;
    }
  }

  std::cout << "Could not find queue family with required properties on "
               "physical device "
            << physicalDevice << std::endl;
  return false;
}

void App::createDevice() {
  uint32_t deviceCount = 0;
  if (vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr) !=
          VK_SUCCESS ||
      deviceCount == 0) {
    throw std::runtime_error(
        "Error occurred during physical device enumeration");
  }

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  if (vkEnumeratePhysicalDevices(this->instance, &deviceCount,
                                 physicalDevices.data()) != VK_SUCCESS) {
    throw std::runtime_error(
        "Error occurred during physical device enumeration");
  }

  uint32_t selectedQueueFamilyIndex = UINT32_MAX;
  for (uint32_t i = 0; i < deviceCount; i++) {
    if (checkPhysicalDeviceProperties(physicalDevices[i],
                                      &selectedQueueFamilyIndex)) {
      physicalDevice = physicalDevices[i];
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error(
        "Could not select physical device based on the chosen properties");
  }

  std::vector<float> queuePriorities = {1.0f};

  VkDeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.pNext = nullptr;
  queueCreateInfo.flags = 0;
  queueCreateInfo.queueFamilyIndex = selectedQueueFamilyIndex;
  queueCreateInfo.queueCount = static_cast<uint32_t>(queuePriorities.size());
  queueCreateInfo.pQueuePriorities = queuePriorities.data();

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = nullptr;
  deviceCreateInfo.flags = 0;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

#ifdef NDEBUG
  deviceCreateInfo.enabledLayerCount = 0;
  deviceCreateInfo.ppEnabledLayerNames = nullptr;
#else
  deviceCreateInfo.enabledLayerCount =
      static_cast<uint32_t>(REQUIRED_VALIDATION_LAYERS.size());
  deviceCreateInfo.ppEnabledLayerNames = REQUIRED_VALIDATION_LAYERS.data();
#endif

  deviceCreateInfo.enabledExtensionCount = 0;
  deviceCreateInfo.ppEnabledExtensionNames = nullptr;

  deviceCreateInfo.pEnabledFeatures = nullptr;

  if (vkCreateDevice(this->physicalDevice, &deviceCreateInfo, nullptr,
                     &this->device) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create logical device");
  }

  this->queueFamilyIndex = selectedQueueFamilyIndex;

  vkGetDeviceQueue(this->device, this->queueFamilyIndex, 0, &this->queue);
}
