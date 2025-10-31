dir = include_directories('.')
add_project_arguments('-D_POSIX_C_SOURCE=200112L', language: 'c')
add_project_arguments('-D_POSIX_C_SOURCE=200112L', language: 'cpp')
cc = meson.get_compiler('c')

fossil_image_lib = library('fossil_image',
    files(
        'process.c',
        'analyze.c',
        'filter.c',
        'color.c',
        'draw.c',
        'io.c'
        ),
    install: true,
    dependencies: [cc.find_library('m', required: false)],
    include_directories: dir)

fossil_image_dep = declare_dependency(
    link_with: [fossil_image_lib],
    include_directories: dir)

meson.override_dependency('fossil-image', fossil_image_dep)