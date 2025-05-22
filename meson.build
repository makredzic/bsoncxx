project(
  'mongocxx',
  'cpp',
  version: '4.1.0'
)

cpp = meson.get_compiler('cpp')

mongocxx_lib = cpp.find_library('mongocxx', dirs: meson.current_source_dir() + '/lib')
mongocxx_hdr = include_directories('include')

mongocxx_dep = declare_dependency(
  dependencies: mongocxx_lib,
  include_directories: mongocxx_hdr,
)

meson.override_dependency('mongocxx', mongocxx_dep)


