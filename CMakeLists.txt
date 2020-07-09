cmake_minimum_required(VERSION 3.5.1)

project(PPCA_RISCV)

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-O2)

add_executable(code RISCV_simulator.cpp)