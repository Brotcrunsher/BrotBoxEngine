cmake_minimum_required(VERSION 3.12.4 FATAL_ERROR)

add_library(GlobalKeyboardDLL SHARED)
target_sources(GlobalKeyboardDLL PUBLIC Main.cpp)

start_embed_file(outputCpp outputH EmbeddedDlls.h)
embed_file_to_source(GlobalKeyboardDLL.dll outputCpp outputH)
end_embed_file(outputCpp outputH EmbeddedDlls.cpp EmbeddedDlls.h)

add_dependencies(BrotBoxEngine GlobalKeyboardDLL)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/EmbeddedDlls.cpp)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/GlobalKeyboard.cpp)
target_include_directories(BrotBoxEngine PUBLIC .)
