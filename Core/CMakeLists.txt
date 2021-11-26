cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)

project(Core VERSION 1.0 DESCRIPTION "Core library")

include(external/nlohmann_json.cmake)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE .)

set_target_properties(Core PROPERTIES 
                VERSION ${PROJECT_VERSION})

target_link_libraries(${PROJECT_NAME} INTERFACE nlohmann_json::nlohmann_json)