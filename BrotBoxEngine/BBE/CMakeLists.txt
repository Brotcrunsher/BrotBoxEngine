if(BBE_RENDER_MODE STREQUAL "Vulkan")
  add_subdirectory(Vulkan)
endif()

file(GLOB local_src CONFIGURE_DEPENDS "*.h")
target_sources(BrotBoxEngine PRIVATE "${local_src}")

