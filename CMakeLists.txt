cmake_minimum_required(VERSION 3.15)

project("DMP")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SQLite3 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(dgdb src/main.cpp src/DGDB.cpp src/network/Network.cpp)
target_link_libraries(dgdb pthread sqlite3)
