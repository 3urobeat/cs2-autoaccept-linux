cmake_minimum_required(VERSION 3.11.0)
project(cs2-autoaccept-linux)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

add_executable(cs2-autoaccept-linux main.cpp)

find_package(X11 COMPONENTS Xtst)
target_link_libraries(cs2-autoaccept-linux ${X11_LIBRARIES} ${X11_Xtst_LIB})
