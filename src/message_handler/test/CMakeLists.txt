include(GoogleTest)

set(TARGET_NAME "message_handler_tests")
find_package(GTest)
file(GLOB SOURCES "*.cpp" "../src/*.cpp")

add_executable(${TARGET_NAME} ${SOURCES})

target_include_directories(${TARGET_NAME}
    PRIVATE "${CMAKE_SOURCE_DIR}/src/data_manager/include"
    PRIVATE "${CMAKE_SOURCE_DIR}/src/message_handler/include" 
    PRIVATE "${CMAKE_SOURCE_DIR}/src/message_handler/src"
)

target_link_libraries(${TARGET_NAME} PRIVATE GTest::GTest)

gtest_discover_tests(${TARGET_NAME})
