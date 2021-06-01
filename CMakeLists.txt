cmake_minimum_required(VERSION 3.15)

project("DMP")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(server server.cpp ${DMP_SOURCES})
target_link_libraries(server pthread)

add_executable(client client.cpp ${DMP_SOURCES})
target_link_libraries(client pthread)