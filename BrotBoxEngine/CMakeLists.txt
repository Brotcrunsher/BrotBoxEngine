add_subdirectory(BBE)
add_subdirectory(Vulkan)

file(GLOB local_src CONFIGURE_DEPENDS "*.cpp")
target_sources(BrotBoxEngine PRIVATE "${local_src}")

target_include_directories(BrotBoxEngine PUBLIC .)
