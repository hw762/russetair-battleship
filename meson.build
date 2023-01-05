project('russetair-battleship', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=gnu17'])

cc = meson.get_compiler('c')

add_global_arguments([
  '-Werror=incompatible-pointer-types',
  '-Werror=uninitialized',
  '-Werror=return-type',
  '-Wno-missing-field-initializers',
  '-Wno-newline-eof',
  ], language: 'c')

cmake = import('cmake')

# Graphics
vulkan_dep = dependency('vulkan')
sdl2_dep = dependency('sdl2')
cglm_dep = dependency('cglm')

gfx_deps = [vulkan_dep, sdl2_dep, cglm_dep]

# Assimp model loader
assimp_dep = dependency('assimp')

# Others
m_dep = cc.find_library('m', required : false)

subdir('src')
flecs_subproject = subproject('flecs', default_options: ['default_library=static'])
flecs_dep = flecs_subproject.get_variable('flecs_dep')
thirdparty_src = files(
  'thirdparty/stb/stb.c',
  'thirdparty/nuklear/nuklear.c',
)

src_inc = include_directories('src')
thirdparty_inc = include_directories([
  'thirdparty/stb',
  'thirdparty/nuklear',
])

exe = executable('c_battleship', [
    'main.c', 
    src,
    thirdparty_src
  ],
  dependencies : [
    gfx_deps,
    flecs_dep,
    assimp_dep,
    m_dep,
  ],
  include_directories : [
    src_inc,
    thirdparty_inc
  ],
  c_args : [
    '-DMESON_PROJECT_NAME="@0@"'.format(meson.project_name())
  ],
  install : true)

test('basic', exe)
