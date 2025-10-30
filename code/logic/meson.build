dir = include_directories('.')
add_project_arguments('-D_POSIX_C_SOURCE=200112L', language: 'c')
add_project_arguments('-D_POSIX_C_SOURCE=200112L', language: 'cpp')

fossil_image_lib = library('fossil_image',
    files(
        'process.c'
        ),
    install: true,
    include_directories: dir)

fossil_image_dep = declare_dependency(
    link_with: [fossil_image_lib],
    include_directories: dir)

meson.override_dependency('fossil-image', fossil_image_dep)