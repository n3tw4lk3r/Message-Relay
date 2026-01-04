cmake_minimum_required(VERSION 3.16)

project(TCP_Project
    VERSION 1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -Wall
    -Wextra
    -Werror
)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src)
