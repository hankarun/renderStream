cmake_minimum_required(VERSION 3.14)
project(RaylibTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use FetchContent to download and build raylib
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 4.5.0  # Specify the version you want
)

# Configure raylib build options
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

# Make the raylib library available
FetchContent_MakeAvailable(raylib)

# Create executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/CelestialBody.cpp
    src/CelestialBody.h
)

# Link with raylib
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Copy resources to build directory
add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    COMMENT "Copying resources to build directory"
)

# Windows-specific settings
if(WIN32)
    # Ensure we use Unicode for Windows
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
      # Use console subsystem for easier debugging
    # When you want to switch to a Windows GUI application without console,
    # uncomment the line below and change main() to WinMain() in main.cpp
    # set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    
    # If using Win32, you'll need to link with these too
    if(MINGW)
        target_link_libraries(${PROJECT_NAME} PRIVATE winmm gdi32)
    endif()
endif()
