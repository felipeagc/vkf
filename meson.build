project('vkf', 'cpp')

subdir('shaders')

dependencies = [
  dependency('glfw3'),
  dependency('vulkan'),
  dependency('glm')
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
  'src/vertex_buffer.cpp',
]

executable(
  'vkf',
  sources,
  dependencies: dependencies
)
