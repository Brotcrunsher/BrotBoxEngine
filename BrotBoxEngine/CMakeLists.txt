add_subdirectory(BBE)

file(GLOB local_src CONFIGURE_DEPENDS "*.cpp")
target_sources(BrotboxEngine PRIVATE "${local_src}")

target_include_directories(BrotboxEngine PUBLIC .)

file(GLOB compiled_shaders CONFIGURE_DEPENDS "*.spv")
set(compiled_shaders ${compiled_shaders} CACHE STRING "" FORCE)

