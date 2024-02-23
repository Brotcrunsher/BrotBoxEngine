add_subdirectory(BBE)
if(BBE_RENDER_MODE STREQUAL "Vulkan")
  add_subdirectory(Vulkan)
elseif(BBE_RENDER_MODE STREQUAL "NullRenderer")
  add_subdirectory(NullRenderer)
elseif(BBE_RENDER_MODE STREQUAL "OpenGL"
    OR BBE_RENDER_MODE STREQUAL "Emscripten")
  add_subdirectory(OpenGL)
endif()
add_subdirectory(Assets)

file(GLOB local_src CONFIGURE_DEPENDS "*.cpp")
target_sources(BrotBoxEngine PRIVATE "${local_src}")

target_include_directories(BrotBoxEngine PUBLIC .)

if (WIN32)
  add_subdirectory(GlobalKeyboardDLL)
endif()
