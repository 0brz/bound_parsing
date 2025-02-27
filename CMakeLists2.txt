cmake_minimum_required(VERSION 3.2)

project(bound_parsing)

add_executable(tt
    src/parcel.h
    #parcel.cpp

    src/graph_table.h
    src/builder.h
    src/builder.cpp

    src/test.cpp
)