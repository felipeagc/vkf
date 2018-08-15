#include "material.hpp"
#include "../framework/framework.hpp"

using namespace vkf;

Material::Material(
    Framework *framework,
    VkShaderModule vertexShaderModule,
    VkShaderModule fragmentShaderModule)
    : framework(framework),
      vertexShaderModule(vertexShaderModule),
      fragmentShaderModule(fragmentShaderModule) {
  this->framework->getWindow()->addHandler(this);
}

void Material::bindPipeline(VkCommandBuffer commandBuffer) {
  vkCmdBindPipeline(
      commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);
}

int Material::getAvailableDescriptorSet() {
  for (size_t i = 0; i < this->descriptorSetAvailable.size(); ++i) {
    if (this->descriptorSetAvailable[i]) {
      return i;
    }
  }
  return -1;
}

void Material::onResize(uint32_t width, uint32_t height) {
  if (this->framework->getContext()->getDevice() != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->framework->getContext()->getDevice());

    if (this->pipelineLayout != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(
          this->framework->getContext()->getDevice(),
          this->pipelineLayout,
          nullptr);
      this->pipelineLayout = VK_NULL_HANDLE;
    }

    if (this->pipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(
          this->framework->getContext()->getDevice(), this->pipeline, nullptr);
      this->pipeline = VK_NULL_HANDLE;
    }
  }

  this->createPipelineLayout();
  this->createPipeline();
}
