file(GLOB local_src CONFIGURE_DEPENDS "*.h")
target_sources(BrotBoxEngine PRIVATE "${local_src}")
