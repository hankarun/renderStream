cmake_minimum_required(VERSION 3.14)
project(RaylibTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use FetchContent to download and build raylib
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5  # Specify the version you want
)

# Add ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9b  # Specify the version you want
)
FetchContent_MakeAvailable(imgui)

# Add rlImGui
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG main  # Use main branch as it's actively maintained
)
FetchContent_MakeAvailable(rlimgui)

# Configure raylib build options
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
set(SUPPORT_FILEFORMAT_HDR ON CACHE BOOL "Enable HDR support" FORCE)

# Make the raylib library available
FetchContent_MakeAvailable(raylib)

set(SHADER_FILES
    resources/shaders/basic.fs
    resources/shaders/basic.vs
)

# Create executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/CelestialBody.cpp
    src/CelestialBody.h
    src/OrbitSystem.cpp
    src/Tools.h
    src/Tools.cpp
    ${SHADER_FILES}
)

set_source_files_properties(${SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

# Add include directories for ImGui and rlImGui
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
)

# Create rlImGui sources
set(RLIMGUI_SOURCES
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
    ${rlimgui_SOURCE_DIR}/rlImGui.h
)

# Create ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Add ImGui and rlImGui sources to the target
target_sources(${PROJECT_NAME} PRIVATE ${IMGUI_SOURCES} ${RLIMGUI_SOURCES})

# Link with raylib
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Copy resources to build directory
add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    COMMENT "Copying resources to build directory"
)

# Create videos directory in build directory
add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/videos
    COMMENT "Creating videos directory in build folder"
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
