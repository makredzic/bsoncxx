project(
  'bsoncxx',
  'cpp',
  version: '4.1.0'
)

cpp = meson.get_compiler('cpp')

bsoncxx_lib = cpp.find_library('bsoncxx', dirs: meson.current_source_dir() + '/lib')
bsoncxx_hdr = include_directories('include')

bsoncxx_dep = declare_dependency(
  dependencies: bsoncxx_lib,
  include_directories: bsoncxx_hdr,
)

meson.override_dependency('bsoncxx', bsoncxx_dep)


