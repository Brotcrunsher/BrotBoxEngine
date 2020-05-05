file(GLOB compiled_shaders_local CONFIGURE_DEPENDS "*.png")
list(APPEND compiled_shaders ${compiled_shaders_local})

add_trivial_project(BrotBoxEngineTest)