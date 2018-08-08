vma_inc_dirs = include_directories('include')

vma_dependencies = [
  dependency('vulkan')
]

vma_lib = library(
  'vma',
  'vk_mem_alloc.cpp',
  include_directories: vma_inc_dirs,
  dependencies: vma_dependencies
)

vma_dep = declare_dependency(
  link_with: vma_lib,
  include_directories: vma_inc_dirs,
  dependencies: vma_dependencies
)
