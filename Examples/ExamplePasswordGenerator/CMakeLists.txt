add_executable(ExamplePasswordGenerator
  ExamplePasswordGenerator.cpp
  ${CMAKE_SOURCE_DIR}/Examples/ExampleMother/PasswordGenerator.cpp
)
target_link_libraries(ExamplePasswordGenerator sodium)
target_include_directories(ExamplePasswordGenerator PRIVATE
  ${CMAKE_SOURCE_DIR}/Examples/ExampleMother
)
