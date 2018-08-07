project('vkf', 'cpp', 'c')

subdir('external')
subdir('shaders')

dependencies = [
  dependency('sdl2'),
  dependency('vulkan'),
  dependency('glm'),
  stb_image_dep,
  vma_dep
]

vkf_sources = [
  'src/vulkan_backend.cpp',
  'src/buffer_transfer.cpp',
  'src/standard_material.cpp',
  'src/mesh.cpp',
  'src/main.cpp'
]

executable(
  'vkf',
  vkf_sources,
  dependencies: dependencies,
)
