add_subdirectory(BBE)

file(GLOB local_src CONFIGURE_DEPENDS "*.cpp")
target_sources(BrotBoxEngine PRIVATE "${local_src}")

target_include_directories(BrotBoxEngine PUBLIC .)
