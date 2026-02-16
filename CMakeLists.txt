cmake_minimum_required(VERSION 3.26.3)
project(nonogram-solver)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add cxxopts
include_directories("third_party/cxxopts/include")

# Add OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Add executable
add_executable(solver
    src/main.cpp
    src/controls.cpp
    src/image.cpp
    src/screen.cpp
)

# Link libraries
target_link_libraries(solver
    ${OpenCV_LIBS}
)

# Print build information
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
# message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
# message(STATUS "OpenCV includes: ${OpenCV_INCLUDE_DIRS}")
