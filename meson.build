project('vkintel', 'cpp')

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
  'src/device.cpp',
]

executable(
  'vkintel',
  sources,
  dependencies: dependencies
)
