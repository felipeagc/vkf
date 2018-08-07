#include "standard_material.hpp"
#include <fstream>

using namespace vkf;

std::vector<char> loadShaderCode(const char *filename) {
  std::ifstream file(filename);

  if (file.fail()) {
    throw std::runtime_error(
        "Failed to open \"" + std::string(filename) + "\"");
  }

  std::streampos begin, end;
  begin = file.tellg();
  file.seekg(0, std::ios::end);
  end = file.tellg();

  std::vector<char> result(static_cast<size_t>(end - begin));
  file.seekg(0, std::ios::beg);
  file.read(result.data(), end - begin);
  file.close();

  return result;
}

StandardMaterial::StandardMaterial(VulkanBackend &backend)
    : Material(
          backend,
          backend.createShaderModule(loadShaderCode("shaders/shader.vert.spv")),
          backend.createShaderModule(
              loadShaderCode("shaders/shader.frag.spv"))) {
  this->createDescriptorSetLayout();

  this->createPipeline();

  this->createDescriptorPool();
  this->allocateDescriptorSets();
}

StandardMaterial::~StandardMaterial() {
  if (this->backend.device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->backend.device);

    if (this->pipelineLayout != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(
          this->backend.device, this->pipelineLayout, nullptr);
      this->pipelineLayout = VK_NULL_HANDLE;
    }

    if (this->pipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(this->backend.device, this->pipeline, nullptr);
      this->pipeline = VK_NULL_HANDLE;
    }

    if (this->descriptorPool != VK_NULL_HANDLE) {
      vkDestroyDescriptorPool(
          this->backend.device, this->descriptorPool, nullptr);
      this->descriptorPool = VK_NULL_HANDLE;
    }

    if (this->descriptorSetLayout != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(
          this->backend.device, this->descriptorSetLayout, nullptr);
      this->descriptorSetLayout = VK_NULL_HANDLE;
    }

    if (this->vertexShaderModule != VK_NULL_HANDLE) {
      vkDestroyShaderModule(
          this->backend.device, this->vertexShaderModule, nullptr);
      this->vertexShaderModule = VK_NULL_HANDLE;
    }

    if (this->fragmentShaderModule != VK_NULL_HANDLE) {
      vkDestroyShaderModule(
          this->backend.device, this->fragmentShaderModule, nullptr);
      this->fragmentShaderModule = VK_NULL_HANDLE;
    }
  }
}

void StandardMaterial::bindPipeline(VkCommandBuffer commandBuffer) {
  vkCmdBindPipeline(
      commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);
}

void StandardMaterial::onResize(uint32_t width, uint32_t height) {
  if (this->backend.device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(this->backend.device);

    if (this->pipelineLayout != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(
          this->backend.device, this->pipelineLayout, nullptr);
      this->pipelineLayout = VK_NULL_HANDLE;
    }

    if (this->pipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(this->backend.device, this->pipeline, nullptr);
      this->pipeline = VK_NULL_HANDLE;
    }
  }

  // TODO: recreate pipeline
  this->createPipelineLayout();
  this->createPipeline();
}

int StandardMaterial::getAvailableDescriptorSet() {
  for (size_t i = 0; i < this->descriptorSetAvailable.size(); ++i) {
    if (this->descriptorSetAvailable[i]) {
      return i;
    }
  }
  return -1;
}

VkPipelineLayout StandardMaterial::createPipelineLayout() {
  VkPipelineLayoutCreateInfo layoutCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .setLayoutCount = 1,
      .pSetLayouts = &descriptorSetLayout,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
  };

  VkPipelineLayout pipelineLayout;
  if (vkCreatePipelineLayout(
          this->backend.device, &layoutCreateInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("Could not create pipeline layout");
  }

  return pipelineLayout;
}

void StandardMaterial::createPipeline() {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos = {
      {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .stage = VK_SHADER_STAGE_VERTEX_BIT,
          .module = this->vertexShaderModule,
          .pName = "main",
          .pSpecializationInfo = nullptr,
      },
      {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
          .module = this->fragmentShaderModule,
          .pName = "main",
          .pSpecializationInfo = nullptr,
      },
  };

  std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions = {
      {
          .binding = 0,
          .stride = sizeof(StandardVertex),
          .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
      },
  };

  auto vertexAttributeDescriptions =
      StandardVertex::getVertexAttributeDescriptions(
          vertexBindingDescriptions[0].binding);

  VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .vertexBindingDescriptionCount =
          static_cast<uint32_t>(vertexBindingDescriptions.size()),
      .pVertexBindingDescriptions = vertexBindingDescriptions.data(),
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(vertexAttributeDescriptions.size()),
      .pVertexAttributeDescriptions = vertexAttributeDescriptions.data(),
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
      .primitiveRestartEnable = VK_FALSE,
  };

  VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .viewportCount = 1,
      .pViewports = nullptr,
      .scissorCount = 1,
      .pScissors = nullptr,
  };

  std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  };

  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };

  VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .depthBiasConstantFactor = 0.0f,
      .depthBiasClamp = 0.0f,
      .depthBiasSlopeFactor = 0.0f,
      .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 1.0f,
      .pSampleMask = nullptr,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE,
  };

  VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
      .blendEnable = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

  VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachmentState,
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
  };

  this->pipelineLayout = this->createPipelineLayout();

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size()),
      .pStages = shaderStageCreateInfos.data(),
      .pVertexInputState = &vertexInputStateCreateInfo,
      .pInputAssemblyState = &inputAssemblyStateCreateInfo,
      .pTessellationState = nullptr,
      .pViewportState = &viewportStateCreateInfo,
      .pRasterizationState = &rasterizationStateCreateInfo,
      .pMultisampleState = &multisampleStateCreateInfo,
      .pDepthStencilState = nullptr,
      .pColorBlendState = &colorBlendStateCreateInfo,
      .pDynamicState = &dynamicStateCreateInfo,
      .layout = this->pipelineLayout,
      .renderPass = this->backend.renderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
  };

  if (vkCreateGraphicsPipelines(
          this->backend.device,
          VK_NULL_HANDLE,
          1,
          &pipelineCreateInfo,
          nullptr,
          &this->pipeline) != VK_SUCCESS) {
    vkDestroyPipelineLayout(
        this->backend.device, this->pipelineLayout, nullptr);
    throw std::runtime_error("Failed to create graphics pipeline");
  }
}

void StandardMaterial::createDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding layoutBinding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .pImmutableSamplers = nullptr,
  };

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = 1,
      .pBindings = &layoutBinding,
  };

  if (vkCreateDescriptorSetLayout(
          this->backend.device,
          &descriptorSetLayoutCreateInfo,
          nullptr,
          &this->descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout");
  }
}

void StandardMaterial::createDescriptorPool() {
  VkDescriptorPoolSize poolSize = {
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
  };

  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .maxSets = MAX_DESCRIPTOR_SETS,
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize,
  };

  if (vkCreateDescriptorPool(
          this->backend.device,
          &descriptorPoolCreateInfo,
          nullptr,
          &this->descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool");
  }
}

void StandardMaterial::allocateDescriptorSets() {
  this->descriptorSetAvailable.fill(true);

  std::array<VkDescriptorSetLayout, MAX_DESCRIPTOR_SETS> setLayouts;
  setLayouts.fill(this->descriptorSetLayout);

  VkDescriptorSetAllocateInfo allocateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = nullptr,
      .descriptorPool = this->descriptorPool,
      .descriptorSetCount = MAX_DESCRIPTOR_SETS,
      .pSetLayouts = setLayouts.data(),
  };

  if (vkAllocateDescriptorSets(
          this->backend.device, &allocateInfo, this->descriptorSets.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor set");
  }
}
