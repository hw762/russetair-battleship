project('russetair-battleship', 'c', 'cpp',
  version : '0.1',
  default_options : [
    'warning_level=3',
    'c_std=gnu17',
    'cpp_std=c++17',
    'default_library=static',
    ])

cc = meson.get_compiler('c')

if cc.get_id() != 'msvc'
  add_global_arguments([
    '-Werror=incompatible-pointer-types',
    '-Werror=uninitialized',
    '-Werror=return-type',
    '-Wno-missing-field-initializers',
    '-Wno-newline-eof',
    '-Wno-nullability-extension'
    ], language: 'c')
endif

add_global_arguments([
  '-DNK_INCLUDE_FIXED_TYPES',
  '-DNK_INCLUDE_DEFAULT_ALLOCATOR',
  '-DNK_INCLUDE_STANDARD_IO',
  '-DNK_INCLUDE_VERTEX_BUFFER_OUTPUT',
  '-DNK_INCLUDE_FONT_BAKING',
  '-DNK_INCLUDE_DEFAULT_FONT',
  '-DNK_ZERO_COMMAND_MEMORY',
], language: 'c')

cmake = import('cmake')

# Graphics
vulkan_dep = dependency('vulkan')
sdl2_dep = dependency('sdl2')
cglm_dep = dependency('cglm')

gfx_deps = [vulkan_dep, sdl2_dep, cglm_dep]

# Assimp model loader
assimp_sp = cmake.subproject('assimp')
assimp_dep = assimp_sp.dependency('assimp')

# Others
m_dep = cc.find_library('m', required : false)

subdir('src/engine')
flecs_subproject = subproject('flecs', default_options: ['default_library=static', 'warning_level=0'])
flecs_dep = flecs_subproject.get_variable('flecs_dep')
thirdparty_src = files(
  'thirdparty/stb/stb.c',
  'thirdparty/nuklear/nuklear.c',
  'thirdparty/vma/vk_mem_alloc.cpp',
)

src_inc = include_directories('src')
thirdparty_inc = include_directories([
  'thirdparty/stb',
  'thirdparty/nuklear',
  'thirdparty/vma'
])

exe = executable('c_battleship', [
    'main.c',
    engine_src,
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
