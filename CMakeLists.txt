cmake_minimum_required(VERSION 3.16.2 FATAL_ERROR)
project(BrotboxEngine)

macro(install_compiled_shaders currProject)
  foreach(shader_path ${compiled_shaders})
    get_filename_component(shader ${shader_path} NAME)
    configure_file("${shader_path}" "${CMAKE_BINARY_DIR}/${currProject}/${shader}" COPYONLY)
  endforeach(shader_path)
endmacro()

macro(add_trivial_project name)
  add_executable(${name})
  target_link_libraries(${name} BrotboxEngine)
  file(GLOB local_src CONFIGURE_DEPENDS "*.cpp")
  target_sources(${name} PUBLIC "${local_src}")
  install_compiled_shaders(${name})
endmacro()

add_library(BrotboxEngine STATIC)

find_package(Vulkan REQUIRED)
target_include_directories(BrotboxEngine PUBLIC Vulkan::Vulkan)
target_link_libraries(BrotboxEngine Vulkan::Vulkan)

add_subdirectory(BrotboxEngine)
add_subdirectory(Third-Party/glfw-3.3.1)
include_directories(Third-Party/glfw-3.3.1/include)
target_link_libraries(BrotboxEngine glfw ${GLFW_LIBRARIES})

target_include_directories(BrotboxEngine PUBLIC "Third-Party/Vulkan/Include")
target_include_directories(BrotboxEngine PUBLIC "Third-Party/stb")

add_subdirectory(Example3D)
add_subdirectory(ExampleMandelbrot)
add_subdirectory(ExampleParticleGravity)
add_subdirectory(ExampleSnake)



