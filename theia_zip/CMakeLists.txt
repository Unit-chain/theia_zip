cmake_minimum_required(VERSION 3.25)
project(theia_zip)

set(CMAKE_CXX_STANDARD 20)

add_executable(theia_zip main.cpp zip.cpp zip.h)
