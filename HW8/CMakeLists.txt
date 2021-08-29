cmake_minimum_required(VERSION 3.5)

project(bulk_multithread)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
add_executable(bulk_multithread main.cpp CommandBlock.h)
#add_executable(test tests.cpp CommandBlock.h)

set(TARGETS bulk_multithread)

foreach(target ${TARGETS})
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
            CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE "-g;-Wall;-Wextra;-Werror;-pedantic")
        target_link_libraries(${target} PUBLIC pthread)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${target} PRIVATE "/W4;/WX")
    endif()
    target_compile_features(${target} PUBLIC cxx_std_17)
endforeach(target IN ${TARGETS})
