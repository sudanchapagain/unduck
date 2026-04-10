project(
  'unduck',
  'cpp',
  version: '1.0.0',
  default_options: [
    'c_std=c++11',
  ]
)

sources = files(
  'src/main.cpp'
)

install_data(
  'data/source.json',
  install_dir: get_option('bindir')
)

out = executable(
  'unduck',
  sources,
  install: true
)
