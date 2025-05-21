cmake_minimum_required(VERSION 3.20)

project(IPQ VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ipq INTERFACE)

target_include_directories(ipq INTERFACE include)
target_compile_options(ipq INTERFACE -Wall -Wextra -Wpedantic)

include(CTest)
enable_testing()

file(GLOB TEST_SOURCES
  test/TestSPSCQueue.cpp
  # test/TestSPMCQueue.cpp
  # test/TestMPSCQueue.cpp
  # test/TestMPMCQueue.cpp
)

foreach(TEST_SOURCE ${TEST_SOURCES})
  get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
  add_executable(${TEST_NAME} ${TEST_SOURCE})
  target_link_libraries(${TEST_NAME} PRIVATE ipq)
  target_compile_options(${TEST_NAME} PRIVATE -Wall -Wextra -Werror -Wpedantic)
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

install(DIRECTORY include DESTINATION include)