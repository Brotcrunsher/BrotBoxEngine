file(GLOB local_src CONFIGURE_DEPENDS "*.cpp")
target_sources(BrotBoxEngine PRIVATE "${local_src}")

target_include_directories(BrotBoxEngine PUBLIC .)

start_embed_file(outputCpp outputH EmbeddedFonts.h)
foreach(filename OpenSansRegular)
  embed_file_to_source(${filename}.ttf outputCpp outputH)
endforeach()
end_embed_file(outputCpp outputH EmbeddedFonts.cpp EmbeddedFonts.h)
