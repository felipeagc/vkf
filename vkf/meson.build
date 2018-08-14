vkf_inc_dirs = include_directories('.')

vkf_sources = [
  'window/window.cpp',

  'renderer/vk_context.cpp',

  'framework/framework.cpp',

  'buffer/buffer.cpp',
  'buffer/staging_buffer.cpp',
  'buffer/vertex_buffer.cpp',
  'buffer/index_buffer.cpp',
  'buffer/uniform_buffer.cpp',

  'texture/texture.cpp',

  'material/material.cpp',
  'material/standard_material.cpp',

  'camera/camera.cpp',

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
  dependencies: vkf_dependencies
)

vkf_dep = declare_dependency(
  link_with: vkf_lib,
  include_directories: vkf_inc_dirs,
  dependencies: vkf_dependencies
)
