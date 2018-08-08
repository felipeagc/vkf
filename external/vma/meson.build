vma_inc_dirs = include_directories('include')

vma_lib = library(
  'vma',
  'vk_mem_alloc.cpp',
  include_directories: vma_inc_dirs,
  dependencies: dependency('vulkan')
)

vma_dep = declare_dependency(
  link_with: vma_lib,
  include_directories: vma_inc_dirs
)
