cmake_minimum_required(VERSION 3.28)
project(IdPackFile)

set(CMAKE_CXX_STANDARD 23)

add_library(
        packfile
        PackReader.cpp
        PackReader.h
)
