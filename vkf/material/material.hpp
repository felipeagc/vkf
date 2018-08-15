#pragma once

#include "../mesh/vertex.hpp"
#include "../window/window.hpp"
#include "../window/event_handler.hpp"
#include <array>

namespace vkf {
const uint32_t MAX_DESCRIPTOR_SETS = 4096;

class Framework;

class Material : public EventHandler {
  friend class Mesh;

public:
  Material(
      Framework *framework,
      VkShaderModule vertexShaderModule,
      VkShaderModule fragmentShaderModule);

  virtual ~Material(){};

  void bindPipeline(VkCommandBuffer commandBuffer);

  void onResize(uint32_t width, uint32_t height) override;

  // Returns the index of the first available descriptor set
  // Returns -1 if not found
  int getAvailableDescriptorSet();

protected:
  Framework *framework;

  VkShaderModule vertexShaderModule{VK_NULL_HANDLE};
  VkShaderModule fragmentShaderModule{VK_NULL_HANDLE};

  VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
  VkPipeline pipeline{VK_NULL_HANDLE};

  VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
  VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
  std::array<VkDescriptorSet, MAX_DESCRIPTOR_SETS> descriptorSets;
  std::array<bool, MAX_DESCRIPTOR_SETS> descriptorSetAvailable;

  virtual VkPipelineLayout createPipelineLayout() = 0;
  virtual void createPipeline() = 0;
  virtual void createDescriptorSetLayout() = 0;
  virtual void createDescriptorPool() = 0;
  virtual void allocateDescriptorSets() = 0;
};
} // namespace vkf
