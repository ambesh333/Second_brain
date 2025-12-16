cmake_minimum_required(VERSION 3.10)

project(LRU_K)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(lru_k src/main.cpp src/lru_k.cpp)