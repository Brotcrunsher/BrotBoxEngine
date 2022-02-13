file(GLOB compiled_shaders_local CONFIGURE_DEPENDS "*.spv")
list(APPEND compiled_shaders ${compiled_shaders_local})

add_trivial_project(ExampleSignedDistanceField)