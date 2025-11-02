CMAKE_MINIMUM_REQUIRED(VERSION 3.22)

PROJECT(1C_TEST_TASK)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_CXX_COMPILER clang++)
SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_FLAGS "-O2")

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/)

ADD_LIBRARY(options STATIC src/options.cpp)
LINK_LIBRARIES(options)

ADD_LIBRARY(get_buff_size STATIC src/get_buff_size.cpp)

ADD_EXECUTABLE(tcp_server src/tcp_server.cpp)
TARGET_LINK_LIBRARIES(tcp_server PRIVATE get_buff_size)
ADD_EXECUTABLE(tcp_client src/tcp_client.cpp)
TARGET_LINK_LIBRARIES(tcp_client PRIVATE get_buff_size)


ADD_EXECUTABLE(udp_server src/udp_server.cpp)
ADD_EXECUTABLE(udp_client src/udp_client.cpp)
