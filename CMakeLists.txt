cmake_minimum_required(VERSION 2.8.4)
project(test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES "Phase 1-1/icmp.c")
add_executable(test ${SOURCE_FILES})