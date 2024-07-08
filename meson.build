project(
    'mlbc', ['cpp', 'c'],
    version: '1.0.0',
    default_options: ['cpp_std=c++20', 'c_std=c99']
)

# Define project sources.
sources = files(
    'src/main.cpp',
    'src/util.cpp',
    'src/colors.cpp',
)


# OpenGL dependencies.
opengl_include_directories = include_directories('vendors/glad/include')
opengl_sources = files('vendors/glad/src/glad.c')
opengl_dep = declare_dependency(
    include_directories: opengl_include_directories,
    sources: opengl_sources,
    dependencies: [
        dependency('gl'),

        # macOS specific dependencies.
        dependency('cocoa'),
        dependency('iokit')
    ]
)

# GLFW dependencies.
glfw_include_directories = include_directories('vendors/glfw/include')
glfw_lib_directory = join_paths(meson.current_source_dir(), 'vendors/glfw/lib-arm64')
glfw_lib_dep = meson.get_compiler('cpp').find_library(
    'glfw3',
    dirs: glfw_lib_directory,
    static: true
)
glfw_dep = declare_dependency(
    include_directories: glfw_include_directories,
    dependencies: glfw_lib_dep
)

# GLM dependencies.
glm_include_directories = include_directories('vendors/glm')
glm_dep = declare_dependency(
    include_directories: glm_include_directories
)

# stb dependencies.
stb_include_directories = include_directories('vendors/stb')
stb_sources = files('vendors/stb/stb_image.cpp')
stb_dep = declare_dependency(
    include_directories: stb_include_directories,
    sources: stb_sources
)

# ImGui dependencies.
imgui_include_directories = include_directories(
    'vendors/imgui',
    'vendors/imgui/backends'
)
imgui_sources = files(
    'vendors/imgui/imgui.cpp',
    'vendors/imgui/imgui_widgets.cpp',
    'vendors/imgui/imgui_tables.cpp',
    'vendors/imgui/imgui_draw.cpp',
    'vendors/imgui/imgui_demo.cpp',

    'vendors/imgui/backends/imgui_impl_glfw.cpp',
    'vendors/imgui/backends/imgui_impl_opengl3.cpp'
)
imgui_dep = declare_dependency(
    include_directories: imgui_include_directories,
    sources: imgui_sources
)

# SFML dependencies.
sfml_include_directories = include_directories('vendors/sfml/include')
sfml_lib_directory = join_paths(meson.current_source_dir(), 'vendors/sfml/lib')
sfml_lib_names = ['sfml-system', 'sfml-audio']
sfml_lib_deps = []
foreach sfml_lib_name : sfml_lib_names
    sfml_lib_deps += meson.get_compiler('cpp').find_library(
        sfml_lib_name,
        dirs: sfml_lib_directory,
        static: true
    )
endforeach

# Other external libraries SFML depends on.
sfml_ext_lib_names = ['openal', 'flac', 'freetype', 'ogg', 'vorbis', 'vorbisenc', 'vorbisfile']
sfml_ext_lib_deps = []
foreach sfml_ext_lib_name : sfml_ext_lib_names
    sfml_ext_lib_deps += dependency(
        sfml_ext_lib_name,
        static: true
    )
endforeach

# Package all sfml dependencies together.
sfml_dep = declare_dependency(
    include_directories: sfml_include_directories,
    dependencies: (sfml_lib_deps + sfml_ext_lib_deps)
)

# JSON dependency.
json_include_directories = include_directories('vendors/json')
json_dep = declare_dependency(
    include_directories: json_include_directories
)

# Package all dependencies together.
dependencies = [
    opengl_dep,
    glfw_dep,
    glm_dep,
    stb_dep,
    imgui_dep,
    sfml_dep,
    json_dep
]

# Create executable.
executable(
    'mlbc',
    sources: sources,
    dependencies: dependencies
)
