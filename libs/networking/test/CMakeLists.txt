include(GoogleTest)

set(TARGET_NAME "tests")
find_package(GTest)
file(GLOB SOURCES "*.cpp")

add_executable(${TARGET_NAME} ${SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/libs/networking")
target_link_libraries(${TARGET_NAME} PRIVATE libnetworking GTest::GTest)

gtest_discover_tests(${TARGET_NAME})
