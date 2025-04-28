cmake_minimum_required(VERSION 3.12)
project(NFunctionPlotter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable GLM experimental features
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

# Set path to the static library
set(NMATH_LIB_DIR ${CMAKE_SOURCE_DIR}/libs)
set(NMATH_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/include)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLM REQUIRED)

# Download ImGui
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui)

# ImGui source files
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Add executable
add_executable(${PROJECT_NAME} 
    nfunction_app.cpp
    function_utils.cpp
    windows_helper.cpp
    shaders_utils.cpp
    vbo_objects.cpp
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${GLM_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${NMATH_INCLUDE_DIR}  # Add the nmath2 headers
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    OpenGL::GL
    GLEW::GLEW
    glfw
    ${NMATH_LIB_DIR}/libnmath2.a  # Add the static library
)

# For MacOS, we need to specify this to avoid linker errors
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# Copy shader files to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders/ DESTINATION ${CMAKE_BINARY_DIR}/shaders/
    FILES_MATCHING PATTERN "*.glsl") 