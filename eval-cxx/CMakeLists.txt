cmake_minimum_required(VERSION 3.10.0)
project(eval-cxx VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CXX_WARNINGS "")
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CXX_OPTIMIZE "-Og")
  set(CXX_SANITIZERS "-fsanitize=address -fsanitize=undefined")
else()
  set(CXX_OPTIMIZE "-O2")
  set(CXX_SANITIZERS "")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_WARNINGS} ${CXX_SANITIZERS}")

add_executable(${PROJECT_NAME} main.cxx)
