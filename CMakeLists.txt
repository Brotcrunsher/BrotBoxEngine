cmake_minimum_required(VERSION 3.12.4 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(BrotboxEngine CXX)

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
set_property(TARGET BrotboxEngine PROPERTY CXX_STANDARD 17)
set_property(TARGET BrotboxEngine PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET BrotboxEngine PROPERTY CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
target_include_directories(BrotboxEngine PUBLIC Vulkan::Vulkan)
target_link_libraries(BrotboxEngine Vulkan::Vulkan)

add_subdirectory(BrotBoxEngine)
add_subdirectory(Third-Party/glfw-3.3.1)
include_directories(Third-Party/glfw-3.3.1/include)
target_link_libraries(BrotboxEngine glfw ${GLFW_LIBRARIES})

target_include_directories(BrotboxEngine PUBLIC "Third-Party/Vulkan/Include")
target_include_directories(BrotboxEngine PUBLIC "Third-Party/stb")

add_subdirectory(BrotBoxEngineTest)
add_subdirectory(BrotBoxEngineGTest)
add_subdirectory(Example3D)
add_subdirectory(ExampleMandelbrot)
add_subdirectory(ExampleMandelbrotShader)
add_subdirectory(ExampleParticleGravity)
add_subdirectory(ExampleSnake)
add_subdirectory(ExampleCurves)
add_subdirectory(ExampleTextRendering)


