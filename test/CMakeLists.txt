cmake_minimum_required(VERSION 3.15)
project(test)

set(CMAKE_CXX_STANDARD 17)
message(STATUS ${CMAKE_BUILD_TYPE})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -DDEBUG")
include_directories(../net)

# aux_source_directory(. SRC_LIST)
link_directories(/usr/lib/mymuduo)
add_executable(test test.cc)
target_link_libraries(test PRIVATE mymuduo)
set_target_properties(test PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                      ${CMAKE_SOURCE_DIR}/bin)
