cmake_minimum_required(VERSION 3.15)
project(EchoServer)

set(CMAKE_CXX_STANDARD 17)
message(STATUS ${CMAKE_BUILD_TYPE})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
include_directories(../net)

# aux_source_directory(. SRC_LIST)
link_directories(/usr/lib/mymuduo)
add_executable(echoserver EchoServer.cc)
target_link_libraries(echoserver PRIVATE mymuduo)
set_target_properties(echoserver PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                            ${CMAKE_SOURCE_DIR}/bin)

add_executable(client Client.cc)
set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                        ${CMAKE_SOURCE_DIR}/bin)
