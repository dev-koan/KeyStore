cmake_minimum_required(VERSION 3.20)
project(KeyStore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB KEYSTORE_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(runFunc ${KEYSTORE_SOURCES} test.cpp)
target_include_directories(runFunc PRIVATE include)

add_executable(runB ${KEYSTORE_SOURCES} benchmark.cpp)
target_include_directories(runB PRIVATE include)

find_package(Threads REQUIRED)
target_link_libraries(runFunc PRIVATE Threads::Threads)
target_link_libraries(runB PRIVATE Threads::Threads)
