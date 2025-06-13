# Raylib Sample Window

A simple C++ project that demonstrates how to use Raylib with CMake's FetchContent.

## Prerequisites

- CMake (version 3.14 or later)
- A C++ compiler (supporting C++17)
- Git (for FetchContent to clone Raylib)

## Building the Project

1. Create a build directory:
```
mkdir build
cd build
```

2. Configure the project with CMake:
```
cmake ..
```

3. Build the project:
```
cmake --build . --config Release
```

4. Run the executable:
```
.\Release\RaylibTest.exe
```

## Project Structure

- `src/main.cpp` - Main application code that creates a window using Raylib
- `CMakeLists.txt` - CMake configuration file that handles downloading and building Raylib

## Notes

- The project uses CMake's FetchContent to automatically download and build Raylib
- Raylib version 4.5.0 is used in this project
- The window is configured to be 800x600 pixels
