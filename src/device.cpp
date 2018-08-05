#include "app.hpp"
#include <cstring>
#include <iostream>

using namespace app;

bool App::checkPhysicalDeviceProperties(
    VkPhysicalDevice physicalDevice,
    uint32_t *selectedGraphicsQueueFamilyIndex,
    uint32_t *selectedPresentQueueFamilyIndex) {
  uint32_t extensionCount = 0;
  if (vkEnumerateDeviceExtensionProperties(
          physicalDevice, nullptr, &extensionCount, nullptr) != VK_SUCCESS ||
      extensionCount == 0) {
    std::cout << "Error occurred during physical device " << physicalDevice
              << " extension enumeration" << std::endl;
    return false;
  }

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  if (vkEnumerateDeviceExtensionProperties(
          physicalDevice,
          nullptr,
          &extensionCount,
          availableExtensions.data()) != VK_SUCCESS) {
    std::cout << "Error occurred during physical device " << physicalDevice
              << " extension enumeration" << std::endl;
    return false;
  }

  for (const auto &requiredExtension : REQUIRED_DEVICE_EXTENSIONS) {
    bool found = false;
    for (const auto &extension : availableExtensions) {
      if (strcmp(requiredExtension, extension.extensionName) == 0) {
        found = true;
      }
    }

    if (!found) {
      std::cout << "Physical device " << physicalDevice
                << " doesn't support extension named \"" << requiredExtension
                << "\"" << std::endl;
      return false;
    }
  }

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
  vkGetPhysicalDeviceQueueFamilyProperties(
      physicalDevice, &queueFamilyCount, nullptr);

  if (queueFamilyCount == 0) {
    std::cout << "Physical device " << physicalDevice
              << " doesn't have any queue families" << std::endl;
    return false;
  }

  std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
  std::vector<VkBool32> queuePresentSupport(queueFamilyCount);

  vkGetPhysicalDeviceQueueFamilyProperties(
      physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

  uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
  uint32_t presentQueueFamilyIndex = UINT32_MAX;

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    vkGetPhysicalDeviceSurfaceSupportKHR(
        physicalDevice, i, this->surface, &queuePresentSupport[i]);

    if (queueFamilyProperties[i].queueCount > 0 &&
        queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      if (graphicsQueueFamilyIndex == UINT32_MAX) {
        graphicsQueueFamilyIndex = i;
      }

      if (queuePresentSupport[i]) {
        *selectedGraphicsQueueFamilyIndex = i;
        *selectedPresentQueueFamilyIndex = i;
        return true;
      }
    }
  }

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    if (queuePresentSupport[i]) {
      presentQueueFamilyIndex = i;
      break;
    }
  }

  if (graphicsQueueFamilyIndex == UINT32_MAX ||
      presentQueueFamilyIndex == UINT32_MAX) {
    std::cout << "Could not find queue family with requested properties on "
                 "physical device "
              << physicalDevice << std::endl;
    return false;
  }

  *selectedGraphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
  *selectedPresentQueueFamilyIndex = presentQueueFamilyIndex;

  return true;
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
  if (vkEnumeratePhysicalDevices(
          this->instance, &deviceCount, physicalDevices.data()) != VK_SUCCESS) {
    throw std::runtime_error(
        "Error occurred during physical device enumeration");
  }

  uint32_t selectedGraphicsQueueFamilyIndex = UINT32_MAX;
  uint32_t selectedPresentQueueFamilyIndex = UINT32_MAX;
  for (uint32_t i = 0; i < deviceCount; i++) {
    if (checkPhysicalDeviceProperties(
            physicalDevices[i],
            &selectedGraphicsQueueFamilyIndex,
            &selectedPresentQueueFamilyIndex)) {
      physicalDevice = physicalDevices[i];
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error(
        "Could not select physical device based on the chosen properties");
  }

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::vector<float> queuePriorities = {1.0f};

  queueCreateInfos.push_back(
      {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
       .pNext = nullptr,
       .flags = 0,
       .queueFamilyIndex = selectedGraphicsQueueFamilyIndex,
       .queueCount = static_cast<uint32_t>(queuePriorities.size()),
       .pQueuePriorities = queuePriorities.data()});

  if (selectedPresentQueueFamilyIndex != selectedGraphicsQueueFamilyIndex) {
    queueCreateInfos.push_back(
        {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .pNext = nullptr,
         .flags = 0,
         .queueFamilyIndex = selectedPresentQueueFamilyIndex,
         .queueCount = static_cast<uint32_t>(queuePriorities.size()),
         .pQueuePriorities = queuePriorities.data()});
  }

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = nullptr;
  deviceCreateInfo.flags = 0;
  deviceCreateInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

#ifdef NDEBUG
  deviceCreateInfo.enabledLayerCount = 0;
  deviceCreateInfo.ppEnabledLayerNames = nullptr;
#else
  deviceCreateInfo.enabledLayerCount =
      static_cast<uint32_t>(REQUIRED_VALIDATION_LAYERS.size());
  deviceCreateInfo.ppEnabledLayerNames = REQUIRED_VALIDATION_LAYERS.data();
#endif

  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(REQUIRED_DEVICE_EXTENSIONS.size());
  deviceCreateInfo.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data();

  deviceCreateInfo.pEnabledFeatures = nullptr;

  if (vkCreateDevice(
          this->physicalDevice, &deviceCreateInfo, nullptr, &this->device) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create logical device");
  }

  this->graphicsQueueFamilyIndex = selectedGraphicsQueueFamilyIndex;
  this->presentQueueFamilyIndex = selectedPresentQueueFamilyIndex;
}
