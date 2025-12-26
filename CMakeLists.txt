cmake_minimum_required(VERSION 3.20)

project(IPQ VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ipq INTERFACE)

target_include_directories(ipq INTERFACE include)
target_compile_options(ipq INTERFACE -Wall -Wextra -Wpedantic)

install(DIRECTORY include/ DESTINATION include/ipq)