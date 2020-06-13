include(google_test.cmake)

add_executable(BrotBoxEngineGTest)
target_link_libraries(BrotBoxEngineGTest PRIVATE gtest gtest_main BrotBoxEngine)
file(GLOB local_src CONFIGURE_DEPENDS "*.cpp")
target_sources(BrotBoxEngineGTest PUBLIC "${local_src}")
target_include_directories(BrotBoxEngineGTest PUBLIC .)
