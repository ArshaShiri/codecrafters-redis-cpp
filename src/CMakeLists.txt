include(GoogleTest)

set(TARGET_NAME "redis")
file(GLOB SOURCES 
    "*.cpp"
    "networking/src/*.cpp"
 )

add_subdirectory(networking)
add_executable(${TARGET_NAME} ${SOURCES})
target_include_directories(${TARGET_NAME}
    PRIVATE
    "${CMAKE_SOURCE_DIR}/src/networking/include"
)
