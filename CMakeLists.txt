cmake_minimum_required(VERSION 3.28.3)
project(ThreadPool VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
# 添加 gtest 库
add_subdirectory(./googletest)
include_directories(${PROJECT_SOURCE_DIR}/include ./googletest/include)
link_directories(${PROJECT_SOURCE_DIR}/googletest)

add_library(base
        include/ThreadPool.h
        src/ThreadPool.cpp
)

add_executable(ThreadPool main.cpp)

# 链接thread_lib到可执行文件
target_link_libraries(${PROJECT_NAME}
        gtest
        base
)
