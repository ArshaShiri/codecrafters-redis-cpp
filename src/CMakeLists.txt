include(GoogleTest)

set(TARGET_NAME "redis_server")
file(GLOB SOURCES 
    "*.cpp"
    "${CMAKE_SOURCE_DIR}/src/data_manager/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/message_handler/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/networking/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/rdb_file_handler/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/redis/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/src/*.cpp"
 )

add_executable(${TARGET_NAME} ${SOURCES})
add_subdirectory(message_handler)
add_subdirectory(networking)
add_subdirectory(redis)
add_subdirectory(utils)

target_include_directories(${TARGET_NAME}
    PUBLIC
   "${CMAKE_SOURCE_DIR}/src/data_manager/include"
   "${CMAKE_SOURCE_DIR}/src/message_handler/include"
   "${CMAKE_SOURCE_DIR}/src/networking/include"
   "${CMAKE_SOURCE_DIR}/src/rdb_file_handler/include"
   "${CMAKE_SOURCE_DIR}/src/redis/include"
   "${CMAKE_SOURCE_DIR}/src/utils/include"
)
