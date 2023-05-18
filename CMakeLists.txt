cmake_minimum_required(VERSION 3.23)
project(OPS_EX4)

set(CMAKE_CXX_STANDARD 14)

add_executable(OPS_EX4 server.c client.c reactorAPI.c)
