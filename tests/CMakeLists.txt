cmake_minimum_required(VERSION 3.2)

project(bound_parsing)

include_directories(include)

add_executable(tt

    src/tests/client.cpp
    src/tools/lex.h
    src/tools/logger.h
)