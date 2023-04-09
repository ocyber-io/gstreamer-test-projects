cmake_minimum_required(VERSION 3.24)

project(gstreamer)

set(CMAKE_CXX_STANDARD 17)


find_package(PkgConfig REQUIRED) #finding pkg-config is a helper tool

#using pkg-config to getting Gstreamer
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

#including GStreamer header files directory
include_directories(
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(src src/gstreamer.cpp src/buffer.cpp src/buffer.h)

#linking GStreamer library directorystreaming
link_directories(
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_LIBRARY_DIRS}
)

#linking GStreamer libraries
link_libraries(
        ${GLIB_LIBRARIES}
        ${GSTREAMER_LIBRARIES}
        src
)

add_definitions(${GST_CFLAGS_OTHER})


#building target executable
add_executable(${PROJECT_NAME} main.cpp)



