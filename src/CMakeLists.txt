include(GoogleTest)

set(TARGET_NAME "redis")
file(GLOB SOURCES "*.cpp")

add_executable(${TARGET_NAME} ${SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/libs/networking")
target_link_libraries(${TARGET_NAME} PRIVATE libnetworking)
