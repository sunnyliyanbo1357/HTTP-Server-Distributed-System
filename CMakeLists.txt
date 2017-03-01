cmake_minimum_required(VERSION 3.5)
project(starter_code)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    cp1_checker.py
    echo_client.c
    echo_server.c)

add_executable(starter_code ${SOURCE_FILES})