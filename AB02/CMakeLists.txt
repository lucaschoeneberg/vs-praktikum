cmake_minimum_required(VERSION 3.24)
project(AB02)

set(CMAKE_CXX_STANDARD 23)

add_executable(AB02 main.cpp CGUtilities.h CGUtilities.cpp color.cpp rgbimage.cpp SimpleRayTracer.cpp vector.cpp)
