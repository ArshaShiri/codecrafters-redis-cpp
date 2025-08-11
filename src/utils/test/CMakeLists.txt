include(GoogleTest)

set(TARGET_NAME "utils_tests")
find_package(GTest)
file(GLOB SOURCES
    "*.cpp"
    "../src/*.cpp"
)

add_executable(${TARGET_NAME} ${SOURCES})

target_include_directories(${TARGET_NAME}
    PRIVATE "${CMAKE_SOURCE_DIR}/src/utils/include"
)

target_link_libraries(${TARGET_NAME} PRIVATE GTest::GTest)

gtest_discover_tests(${TARGET_NAME})
