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

option(INA226_BUILD_TESTS "Build host unit tests" OFF)
option(INA226_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(INA226_ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

add_library(ina226_instrumentation INTERFACE)

if(INA226_ENABLE_SANITIZERS)
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "INA226_ENABLE_SANITIZERS is only supported for host builds.")
    endif()
    target_compile_options(ina226_instrumentation INTERFACE
        -fsanitize=address,undefined
        -fno-sanitize-recover=all
        -fno-omit-frame-pointer
    )
    target_link_options(ina226_instrumentation INTERFACE
        -fsanitize=address,undefined
        -fno-sanitize-recover=all
        -fno-omit-frame-pointer
    )
endif()

if(INA226_ENABLE_COVERAGE)
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "INA226_ENABLE_COVERAGE is only supported for host builds.")
    endif()
    target_compile_options(ina226_instrumentation INTERFACE --coverage)
    target_link_options(ina226_instrumentation INTERFACE --coverage)
endif()

if(INA226_ENABLE_SANITIZERS OR INA226_ENABLE_COVERAGE)
    target_link_libraries(ina226 PRIVATE ina226_instrumentation)
endif()

if(INA226_BUILD_TESTS)
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "INA226_BUILD_TESTS is only supported for host builds.")
    endif()
    enable_testing()
    add_subdirectory(tests)
endif()
