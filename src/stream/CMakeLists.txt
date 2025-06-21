# Stream library CMakeLists.txt
cmake_minimum_required(VERSION 3.14)

# Find FFmpeg components
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavutil
    libswscale
)

# Create the stream static library
add_library(stream STATIC
    Stream.cpp
    Stream.h
)

# Include directories for the library
target_include_directories(stream PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FFMPEG_INCLUDE_DIRS}
)

# Link FFmpeg libraries
target_link_libraries(stream PUBLIC PkgConfig::FFMPEG)
