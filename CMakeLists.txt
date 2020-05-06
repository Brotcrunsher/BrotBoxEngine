cmake_minimum_required(VERSION 3.12.4 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

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
  target_include_directories(${name} PUBLIC .)
endmacro()

add_library(BrotboxEngine STATIC)
set_property(TARGET BrotboxEngine PROPERTY CXX_STANDARD 17)
set_property(TARGET BrotboxEngine PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET BrotboxEngine PROPERTY CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(STATUS "Adding sanitizers")
  target_compile_options(BrotboxEngine PUBLIC "-fsanitize=address"
                                              "-fsanitize=pointer-compare"
                                              "-fsanitize=pointer-subtract"
                                              "-fsanitize=undefined"
                                              "-fsanitize=integer-divide-by-zero"
                                              "-fsanitize=unreachable"
                                              "-fsanitize=vla-bound"
                                              "-fsanitize=null"
                                              "-fsanitize=return"
                                              "-fsanitize=signed-integer-overflow"
                                              "-fsanitize=bounds-strict"
                                              "-fsanitize=enum"
                                              "-fsanitize=bool"
                                              "-fsanitize=vptr"
                                              "-fsanitize=pointer-overflow")
endif()

find_package(Vulkan REQUIRED)
target_include_directories(BrotboxEngine PUBLIC Vulkan::Vulkan)
target_link_libraries(BrotboxEngine Vulkan::Vulkan)

add_subdirectory(BrotBoxEngine)
add_subdirectory(Third-Party/glfw-3.3.2)
include_directories(Third-Party/glfw-3.3.2/include)
target_link_libraries(BrotboxEngine glfw ${GLFW_LIBRARIES})

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
add_subdirectory(ExampleBalloonGame)
add_subdirectory(ExampleSingedDistanceField)


