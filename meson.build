project('russetair-battleship', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=gnu17'])

add_global_arguments([
  '-Werror=discarded-qualifiers', 
  '-Werror=incompatible-pointer-types',
  '-Werror=uninitialized'
  ], language: 'c')

cmake = import('cmake')

# Vulkan
vulkan_dep = dependency('vulkan')

# SDL2
sdl2_dep = dependency('sdl2')

# Assimp model loader
assimp_dep = dependency('assimp')

# SQLite3
sqlite3_dep = dependency('sqlite3')

subdir('src')

thirdparty_src = files(
  'thirdparty/flecs/flecs.c',
)

src_inc = include_directories('src')
thirdparty_inc = include_directories(['thirdparty/flecs'])

exe = executable('c_battleship', [
    'main.c', 
    src,
    thirdparty_src
  ],
  dependencies : [vulkan_dep, sdl2_dep, sqlite3_dep],
  include_directories : [
    src_inc,
    thirdparty_inc
  ],
  c_args : [
    '-DMESON_PROJECT_NAME="@0@"'.format(meson.project_name())
  ],
  install : true)

test('basic', exe)
