cmake_minimum_required(VERSION 3.26)
project(llvm_h)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(llvm_h
        llvm.cpp
        llvm.hpp
        test.cpp)
