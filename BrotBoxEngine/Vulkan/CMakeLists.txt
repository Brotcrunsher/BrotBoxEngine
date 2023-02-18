file(GLOB local_src CONFIGURE_DEPENDS "*.cpp")
target_sources(BrotBoxEngine PRIVATE "${local_src}")

target_include_directories(BrotBoxEngine PUBLIC .)

start_embed_file(outputCpp outputH EmbedOutput.h)
foreach(filename frag2DImage;frag2DPrimitive;frag3DPrimitive;vert2DPrimitive;vert3DPrimitive)
  embed_file_to_source(${filename}.spv outputCpp outputH)
endforeach()
end_embed_file(outputCpp outputH EmbedOutput.cpp EmbedOutput.h)
