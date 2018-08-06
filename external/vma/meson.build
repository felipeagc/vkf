vma_lib = library(
  'vma',
  'vk_mem_alloc.cpp',
  include_directories: include_directories('include'),
  dependencies: dependency('vulkan')
)

vma_dep = declare_dependency(
  link_with: vma_lib,
  include_directories: include_directories('include')
)
