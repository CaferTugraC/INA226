set(UNITY_SRC "${CMAKE_CURRENT_SOURCE_DIR}/unity/src/unity.c")

if(NOT EXISTS "${UNITY_SRC}")
    message(FATAL_ERROR
        "Unity test framework not found at ${UNITY_SRC}.\n"
        "Please run: git submodule update --init --recursive"
    )
endif()

add_library(unity STATIC "${UNITY_SRC}")
target_include_directories(unity SYSTEM PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/unity/src"
)

add_executable(test_ina226_runner
    host/test_mock_ina226.c
)

target_include_directories(test_ina226_runner PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_options(test_ina226_runner PRIVATE
    -pedantic
    -Wall
    -Wextra
    -Wformat=2
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wundef
    -Wstrict-prototypes
    -Wvla
    -Wdouble-promotion
)

target_link_libraries(test_ina226_runner PRIVATE
    ina226
    unity
)

if(INA226_ENABLE_SANITIZERS OR INA226_ENABLE_COVERAGE)
    target_link_libraries(test_ina226_runner PRIVATE ina226_instrumentation)
endif()

add_test(
    NAME host_unit_tests
    COMMAND test_ina226_runner
)
