include(google_test.cmake)

add_executable(BrotBoxEngineGTest)
target_link_libraries(BrotBoxEngineGTest PRIVATE gtest gtest_main BrotboxEngine)
file(GLOB local_src CONFIGURE_DEPENDS "*.cpp")
target_sources(BrotBoxEngineGTest PUBLIC "${local_src}")
