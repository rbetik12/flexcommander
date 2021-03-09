cmake_minimum_required(VERSION 3.17)
project(flexcommander C)

add_compile_definitions(HFSPlus)

include_directories("include")
include_directories("src/include")
set(CMAKE_C_STANDARD 11)

add_library(flexcommander SHARED
        include/FlexIO.h
        src/FlexIO.c
        include/Flexcommander.h src/Init.c src/Devices.c include/HFSPlus.h src/FSHFSPlus.c
        include/HFSPlusBTree.h src/BTreePrint.c include/HFSCatalog.h src/BTree.c src/utils/Endians.h src/utils/Endians.c src/HFSCatalog.c src/include/List.h src/include/List.c)