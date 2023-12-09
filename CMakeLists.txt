
set(PROJECT_CXX_STANDARD 17)
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD ${PROJECT_CXX_STANDARD})

project ("huffman")

add_executable (huffman "huffmain.cpp" "huffman.h" "huffman.cpp")
