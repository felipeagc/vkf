#pragma once

#include "../mesh/vertex.hpp"
#include "material.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkf {
class StandardMaterial : public Material {
public:
  StandardMaterial(Framework *framework);
  virtual ~StandardMaterial();

protected:
  VkPipelineLayout createPipelineLayout() override;
  void createPipeline() override;
  void createDescriptorSetLayout() override;
  void createDescriptorPool() override;
  void allocateDescriptorSets() override;
};

} // namespace vkf
