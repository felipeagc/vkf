vkf_inc_dirs = [
  include_directories('.'),
  vma_inc_dirs,
  stb_image_inc_dirs
]

vkf_sources = [
  'renderer/vulkan_backend.cpp',
  'helper/buffer_transfer.cpp',
  'material/standard_material.cpp',
  'window/window.cpp',
  'framework/framework.cpp',
  'mesh/mesh.cpp',
]

vkf_dependencies = [
  dependency('sdl2'),
  dependency('vulkan'),
  dependency('glm'),
  stb_image_dep,
  vma_dep
]

vkf_lib = library(
  'vkf',
  vkf_sources,
  dependencies: vkf_dependencies,
  include_directories: vkf_inc_dirs
)

vkf_dep = declare_dependency(
  link_with: vkf_lib,
  include_directories: vkf_inc_dirs
)
