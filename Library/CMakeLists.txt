cmake_minimum_required(VERSION 3.16)
project(RmqBridge LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_RABBITMQ_C_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_SSL_SUPPORT OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    rabbitmq-c
    GIT_REPOSITORY https://github.com/alanxz/rabbitmq-c.git
    GIT_TAG v0.14.0
)
FetchContent_MakeAvailable(rabbitmq-c)

add_library(RmqBridge SHARED
    src/client.cpp
    src/error.cpp
    src/dll_exports.cpp
)

target_include_directories(RmqBridge PUBLIC include)

target_link_libraries(RmqBridge PRIVATE rabbitmq::rabbitmq-static)

if(WIN32)
    target_link_libraries(RmqBridge PRIVATE ws2_32)
    target_sources(RmqBridge PRIVATE RmqBridge.def)
endif()

set_target_properties(RmqBridge PROPERTIES
    OUTPUT_NAME "RmqBridge"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<IF:$<STREQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>"
)
