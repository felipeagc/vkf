cc = meson.get_compiler('c')

m_dep = cc.find_library('m', required: false)

stb_image_inc_dirs = include_directories('include')

stb_image_lib = library(
  'stb_image',
  'stb_image.c',
  include_directories: stb_image_inc_dirs,
  dependencies: m_dep
)

stb_image_dep = declare_dependency(
  link_with: stb_image_lib,
  include_directories: stb_image_inc_dirs,
  dependencies: m_dep
)
