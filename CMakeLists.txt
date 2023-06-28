cmake_minimum_required(VERSION 3.16)
project(docgen)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_executable(docgen src/main.cc)
target_include_directories(docgen PRIVATE include)