include(GoogleTest)

set(TARGET_NAME "redis")
file(GLOB SOURCES 
    "*.cpp"
    "${CMAKE_SOURCE_DIR}/src/networking/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/message_handler/src/*.cpp"
 )

add_subdirectory(networking)
add_subdirectory(message_handler)
add_executable(${TARGET_NAME} ${SOURCES})
target_include_directories(${TARGET_NAME}
    PRIVATE
    "${CMAKE_SOURCE_DIR}/src/networking/include"
    "${CMAKE_SOURCE_DIR}/src/message_handler/include"
)
