cmake_minimum_required(VERSION 3.21)
project(ina226 VERSION 0.3.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(ina226 STATIC src/ina226.c)

target_include_directories(ina226 PUBLIC
    src
)

target_compile_options(ina226 PRIVATE
    -pedantic
    -Wall
    -Wextra
    -Wconversion
    -Wsign-conversion
)
