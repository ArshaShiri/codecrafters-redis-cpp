cmake_minimum_required(VERSION 3.21)

project(
    messageParsing
    VERSION 1.0
    LANGUAGES CXX
)

set(TARGET_NAME lib${PROJECT_NAME})
add_library(${TARGET_NAME})
file(GLOB SOURCES "src/*.cpp")

target_include_directories(${TARGET_NAME} PUBLIC include)
target_sources(${TARGET_NAME} PRIVATE ${SOURCES})
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)
# add_subdirectory(test)
