cmake_minimum_required(VERSION 3.11.0)
project(cs2-autoaccept-linux)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS "-fpermissive") # Necessary to let grim compile
set(PIXMAN_ROOT_DIR /usr/include/pixman-1)

add_executable(cs2-autoaccept-linux
    include/grim/box.c
	include/grim/buffer.c
	include/grim/output-layout.c
	include/grim/render.c
    include/grim/build/grim.p/ext-image-capture-source-v1-protocol.c
    include/grim/build/grim.p/ext-image-copy-capture-v1-protocol.c
    include/grim/build/grim.p/ext-foreign-toplevel-list-v1-protocol
    include/grim/build/grim.p/wlr-screencopy-unstable-v1-protocol.c
    include/grim/build/grim.p/xdg-output-unstable-v1-protocol.c
    main.cpp
    wayland_display.cpp
    wayland_mouse.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIXMAN REQUIRED pixman-1)
pkg_check_modules(WAYLAND REQUIRED wayland-client)
pkg_check_modules(WLROOTS REQUIRED wlroots-0.18)

include_directories(include/grim include/grim/include include/grim/build/grim.p)

find_package(X11 COMPONENTS Xtst)
target_link_libraries(cs2-autoaccept-linux ${X11_LIBRARIES} ${X11_Xtst_LIB} ${PIXMAN_LIBRARIES} ${WAYLAND_LIBRARIES}) # ${WLROOTS_LIBRARIES}
target_include_directories(cs2-autoaccept-linux PRIVATE ${PIXMAN_INCLUDE_DIRS})
