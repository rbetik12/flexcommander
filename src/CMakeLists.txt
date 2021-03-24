cmake_minimum_required(VERSION 3.17)

set(LIB "../")
include_directories(${LIB}/lib/flexcommander-lib/include)

add_executable(flexcommander Flexcommander.c)
target_link_libraries(flexcommander flexcommander-lib)