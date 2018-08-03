#include "app.hpp"
#include <fstream>

using namespace app;

std::vector<char> loadShaderCode(const char *filename) {
  std::ifstream file(filename);

  if (file.fail()) {
    throw std::runtime_error("Failed to open \"" + std::string(filename) +
                             "\"");
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

VkShaderModule App::createShaderModule(const char *filename) {
  const auto code = loadShaderCode(filename);
  if (code.size() == 0) {
    throw std::runtime_error("Shader code loaded with size 0");
  }

  VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.pNext = nullptr;
  shaderModuleCreateInfo.flags = 0;
  shaderModuleCreateInfo.codeSize = code.size();
  shaderModuleCreateInfo.pCode =
      reinterpret_cast<const uint32_t *>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(this->device, &shaderModuleCreateInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module");
  }

  return shaderModule;
}
