add_subdirectory(BBE)
if(BBE_RENDER_MODE STREQUAL "Vulkan")
  add_subdirectory(Vulkan)
endif()

file(GLOB local_src CONFIGURE_DEPENDS "*.cpp")
target_sources(BrotBoxEngine PRIVATE "${local_src}")

target_include_directories(BrotBoxEngine PUBLIC .)
