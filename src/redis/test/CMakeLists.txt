include(GoogleTest)

set(TARGET_NAME "redis_tests")
find_package(GTest)
file(GLOB SOURCES
    "*.cpp"
    "../src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/data_manager/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/message_handler/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/networking/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/rdb_file_handler/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/src/*.cpp"
)

add_executable(${TARGET_NAME} ${SOURCES})

target_include_directories(${TARGET_NAME}
    PRIVATE "${CMAKE_SOURCE_DIR}/src/data_manager/include" 
    PRIVATE "${CMAKE_SOURCE_DIR}/src/message_handler/include"
    PRIVATE "${CMAKE_SOURCE_DIR}/src/networking/include"
    PRIVATE "${CMAKE_SOURCE_DIR}/src/rdb_file_handler/include"
    PRIVATE "${CMAKE_SOURCE_DIR}/src/redis/include"
    PRIVATE "${CMAKE_SOURCE_DIR}/src/utils/include"
)

target_link_libraries(${TARGET_NAME} PRIVATE GTest::GTest)

gtest_discover_tests(${TARGET_NAME})
