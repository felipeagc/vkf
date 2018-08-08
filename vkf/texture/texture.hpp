#pragma once

#include <stb_image.h>
#include <stdexcept>
#include <vector>
#include <vk_mem_alloc.h>

namespace vkf {
class Framework;

class Texture {
public:
  Texture();
  Texture(Framework *framework, uint32_t width, uint32_t height);
  ~Texture(){};

  void destroy();

  VkImage getImageHandle();
  VkImageView getImageViewHandle();
  VkSampler getSamplerHandle();

  uint32_t getWidth() const;
  uint32_t getHeight() const;

  static std::vector<unsigned char>
  loadDataFromFile(const char *path, uint32_t *width, uint32_t *height) {
    int iwidth, iheight, n_components;
    unsigned char *imageData =
        stbi_load(path, &iwidth, &iheight, &n_components, STBI_rgb_alpha);

    if (imageData == nullptr) {
      throw std::runtime_error("Failed to load texture file");
    }

    *width = static_cast<uint32_t>(iwidth);
    *height = static_cast<uint32_t>(iheight);

    size_t imageSize = (*width) * (*height) * 4;

    std::vector<unsigned char> data(imageData, imageData + imageSize);

    stbi_image_free(imageData);

    return data;
  }

protected:
  Framework *framework{nullptr};

  uint32_t width = 0;
  uint32_t height = 0;

  VkImage image{VK_NULL_HANDLE};
  VkImageView imageView{VK_NULL_HANDLE};
  VkSampler sampler{VK_NULL_HANDLE};
  VmaAllocation imageAllocation{VK_NULL_HANDLE};
};
} // namespace vkf
