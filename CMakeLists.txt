cmake_minimum_required(VERSION 3.28)
project(ConLogParser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/CompilerOptions.cmake)

find_package(TBB REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(src)
