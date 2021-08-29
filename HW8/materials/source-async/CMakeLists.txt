cmake_minimum_required(VERSION 3.5)
project(22_Async)

set(CMAKE_CXX_STANDARD 14)

find_package(Threads)

add_executable(promise promise.cpp)
set_target_properties(promise PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)
target_link_libraries(promise
        ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(thread_pool thread_pool.cpp)
set_target_properties(thread_pool PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
)
target_link_libraries(thread_pool
        ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(threads threads.cpp)
set_target_properties(threads PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
)
target_link_libraries(threads
        ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(cashier_reader cashier_reader.cpp)
set_target_properties(cashier_reader PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
)
target_link_libraries(cashier_reader
        ${CMAKE_THREAD_LIBS_INIT}
)


add_executable(cashier_events cashier_events.cpp)
set_target_properties(cashier_events PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
)
target_link_libraries(cashier_events
        ${CMAKE_THREAD_LIBS_INIT}
)


add_executable(events events.cpp)
set_target_properties(events PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
)
target_link_libraries(events
        ${CMAKE_THREAD_LIBS_INIT}
)
