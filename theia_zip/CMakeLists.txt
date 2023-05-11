cmake_minimum_required(VERSION 3.25)
project(theia_zip)

set(CMAKE_CXX_STANDARD 20)

find_package(zstd CONFIG REQUIRED)

add_library(theia_zip
        zip.h
        ZIPArchive.h
        ZIPArchive.cpp
        crc32.h
)

target_link_libraries(${PROJECT_NAME}
        zstd::libzstd_static
)