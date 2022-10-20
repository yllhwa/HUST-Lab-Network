cmake_minimum_required(VERSION 3.23)
project(tftp_client C)

set(CMAKE_C_STANDARD 99)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

link_libraries(Ws2_32.lib)
add_library(tftp_client SHARED library.c log.c log.h)

add_executable(test test/test.c)
target_link_libraries(test tftp_client)