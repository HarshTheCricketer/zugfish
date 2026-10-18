cmake_minimum_required(VERSION 3.20)
project(zugfish LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Core Library
add_library(zugfish_core STATIC
    core/uci_engine.cpp
    core/pgn_parser.cpp
    core/reviewer.cpp
    core/opening_book.cpp
    core/online_importer.cpp
)
target_include_directories(zugfish_core PUBLIC core third_party)
target_link_libraries(zugfish_core PUBLIC CURL::libcurl nlohmann_json::nlohmann_json)
target_compile_options(zugfish_core PRIVATE -Wall -Wextra)

# CLI Executable
add_executable(zugfish-cli cli/main.cpp)
target_link_libraries(zugfish-cli PRIVATE zugfish_core)

# Dear ImGui Library
add_library(imgui STATIC
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    third_party/imgui
    third_party/imgui/backends
)
target_link_libraries(imgui PUBLIC
    glfw
    OpenGL::GL
)

# GUI Executable
add_executable(zugfish-gui gui/main.cpp)
target_link_libraries(zugfish-gui PRIVATE zugfish_core imgui)