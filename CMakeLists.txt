cmake_minimum_required(VERSION 3.11.0)
project(cs2-autoaccept-linux)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(PIXMAN_ROOT_DIR /usr/include/pixman-1)

add_executable(cs2-autoaccept-linux
    main.cpp
    wayland_display.cpp
    wayland_mouse.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIXMAN REQUIRED pixman-1)
pkg_check_modules(PORTAL REQUIRED libportal)
#pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET "gtk+-2.0")

find_package(X11 COMPONENTS Xtst)
target_link_libraries(cs2-autoaccept-linux ${X11_LIBRARIES} ${X11_Xtst_LIB} ${PIXMAN_LIBRARIES} ${PORTAL_LIBRARIES} PkgConfig::GTK) # ${WLROOTS_LIBRARIES} ${GLIB_LIBRARIES}
target_include_directories(cs2-autoaccept-linux PRIVATE ${PIXMAN_INCLUDE_DIRS})
