cmake_minimum_required(VERSION 3.14)
project(my_project)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
endif()

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES tests/*.test.cpp)
file(GLOB_RECURSE SOURCES srcs/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")

foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src} ${SOURCES})
    target_include_directories(${test_name} PRIVATE includes)
    target_link_libraries(${test_name} GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(${test_name})
endforeach()

enable_testing()
