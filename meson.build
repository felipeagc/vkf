project('vkf', 'cpp', 'c')

subdir('external')
subdir('shaders')

dependencies = [
  dependency('glfw3'),
  dependency('vulkan'),
  dependency('glm'),
  stb_image_dep
]

sources = [
  'src/main.cpp',

  'src/app.cpp',

  'src/instance.cpp',
  'src/validation.cpp',
  'src/extensions.cpp',
  'src/surface.cpp',
  'src/window.cpp',
  'src/device.cpp',
  'src/queue.cpp',
  'src/sync.cpp',
  'src/swapchain.cpp',
  'src/drawing.cpp',
  'src/commands.cpp',
  'src/renderpass.cpp',
  'src/framebuffer.cpp',
  'src/shader.cpp',
  'src/pipeline.cpp',
  'src/buffers.cpp',
  'src/image.cpp',
  'src/descriptors.cpp',
]

executable(
  'vkf',
  sources,
  dependencies: dependencies,
)
