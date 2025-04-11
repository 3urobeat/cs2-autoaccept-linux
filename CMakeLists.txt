cmake_minimum_required(VERSION 3.11.0)
project(csgo-autoaccept-cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

add_executable(csgo-autoaccept-cpp main.cpp)

find_package(X11 COMPONENTS Xtst)
target_link_libraries(csgo-autoaccept-cpp ${X11_LIBRARIES} ${X11_Xtst_LIB})
