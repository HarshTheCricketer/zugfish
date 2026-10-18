cmake_minimum_required(VERSION 3.21)
project(zugfish VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_GUI "Build Qt6 GUI application" ON)
option(BUILD_CLI "Build CLI tool" ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

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
target_compile_options(zugfish_core PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>)

# CLI Executable
if(BUILD_CLI)
    add_executable(zugfish-cli cli/main.cpp)
    target_link_libraries(zugfish-cli PRIVATE zugfish_core)
endif()

# Qt GUI Executable
if(BUILD_GUI)
    add_subdirectory(gui)
endif()

include(GNUInstallDirs)
if(BUILD_CLI)
    install(TARGETS zugfish-cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()