cmake_minimum_required(VERSION 3.17)

set(LIB "../")
include_directories(${LIB}/lib/flexcommander/include)

add_executable(flexcom Flexcommander.c)
target_link_libraries(flexcom flexcommander blkid)