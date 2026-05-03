cmake_minimum_required(VERSION 3.20)
project(polycpp_qs VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Fetch polycpp (disable its tests when building as a dependency)
set(POLYCPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    polycpp
    GIT_REPOSITORY git@github.com:enricohuang/polycpp.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(polycpp)

# Library target
add_library(polycpp_qs
    src/qs.cpp
)
target_link_libraries(polycpp_qs PUBLIC polycpp)
target_include_directories(polycpp_qs PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Alias for consistent naming
add_library(polycpp::qs ALIAS polycpp_qs)

# Examples
option(POLYCPP_QS_BUILD_EXAMPLES "Build examples" OFF)
if(POLYCPP_QS_BUILD_EXAMPLES)
    add_executable(polycpp_qs_example_parse_roundtrip examples/parse_roundtrip.cpp)
    target_link_libraries(polycpp_qs_example_parse_roundtrip PRIVATE polycpp_qs)
    set_target_properties(polycpp_qs_example_parse_roundtrip PROPERTIES
        OUTPUT_NAME parse_roundtrip
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )

    add_executable(polycpp_qs_example_array_formats examples/array_formats.cpp)
    target_link_libraries(polycpp_qs_example_array_formats PRIVATE polycpp_qs)
    set_target_properties(polycpp_qs_example_array_formats PROPERTIES
        OUTPUT_NAME array_formats
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
endif()

# Tests
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(POLYCPP_QS_BUILD_TESTS_DEFAULT ON)
else()
    set(POLYCPP_QS_BUILD_TESTS_DEFAULT OFF)
endif()
option(POLYCPP_QS_BUILD_TESTS "Build tests" ${POLYCPP_QS_BUILD_TESTS_DEFAULT})
if(POLYCPP_QS_BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(test_qs tests/test_qs.cpp)
    target_link_libraries(test_qs PRIVATE polycpp_qs GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(test_qs)
endif()
