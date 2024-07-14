cmake_minimum_required(VERSION 3.10)
project(circle)

set(CMAKE_CXX_STANDARD 11)

find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(circle main.cpp)

target_link_libraries(circle
    PRIVATE glfw
    PRIVATE GLEW
    PRIVATE OpenGL::GL
    PRIVATE glm::glm
)