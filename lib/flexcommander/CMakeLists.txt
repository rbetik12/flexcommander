cmake_minimum_required(VERSION 3.17)
project(flexcommander C)

add_compile_definitions(HFSPlus)

include_directories("include")
set(CMAKE_C_STANDARD 11)

add_library(flexcommander SHARED include/Flexcommander.h src/Init.c src/Devices.c include/HFSPlus.h src/FSHFSPlus.c)