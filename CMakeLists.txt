cmake_minimum_required(VERSION 3.11.0)
project(cs2-autoaccept-linux)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

add_executable(cs2-autoaccept-linux
    src/main.cpp
    src/process_image.cpp
    src/wayland_display.cpp
    src/wayland_mouse.cpp
    src/x11_display.cpp
    src/x11_mouse.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAL REQUIRED libportal)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET "gtk+-3.0")

find_package(PNG REQUIRED)
find_package(X11 COMPONENTS Xtst)

target_link_libraries(cs2-autoaccept-linux PRIVATE ${X11_LIBRARIES} ${X11_Xtst_LIB} ${PORTAL_LIBRARIES} ${PNG_LIBRARIES} PkgConfig::GTK)
