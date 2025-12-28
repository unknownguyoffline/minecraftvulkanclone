cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
project(MinecraftVulkan)

include_directories("${PROJECT_SOURCE_DIR}/Libraries/glfw/include/")
include_directories("${PROJECT_SOURCE_DIR}/Libraries/glm/")
include_directories("${PROJECT_SOURCE_DIR}/Libraries/")
include_directories("${PROJECT_SOURCE_DIR}/Headers/")

find_package(Vulkan REQUIRED)

file(GLOB source_files 
    "${PROJECT_SOURCE_DIR}/Sources/*.cpp"
)

add_subdirectory(Libraries)

add_executable(minevulkan ${source_files})
target_link_libraries(minevulkan glfw)
target_link_libraries(minevulkan stb)
target_link_libraries(minevulkan Vulkan::Vulkan)