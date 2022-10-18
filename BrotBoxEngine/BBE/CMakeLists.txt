if(BBE_RENDER_MODE STREQUAL "Vulkan")
  add_subdirectory(Vulkan)
elseif(BBE_RENDER_MODE STREQUAL "NullRenderer")
  add_subdirectory(NullRenderer)
elseif(BBE_RENDER_MODE STREQUAL "OpenGL"
    OR BBE_RENDER_MODE STREQUAL "Emscripten")
  add_subdirectory(OpenGL)
endif()

file(GLOB local_src CONFIGURE_DEPENDS "*.h")
target_sources(BrotBoxEngine PRIVATE "${local_src}")

