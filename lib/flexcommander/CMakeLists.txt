cmake_minimum_required(VERSION 3.17)
project(flexcommander C)

include_directories("include")
set(CMAKE_C_STANDARD 11)

add_library(flexcommander SHARED include/Flexcommander.h src/Hello.c src/Init.c src/Devices.c)