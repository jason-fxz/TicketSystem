cmake_minimum_required(VERSION 3.0.0)
project(BPlusTree VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -O3")
message(STATUS "CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

include(CTest)
enable_testing()

add_executable(code prework/BPlusTree.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
