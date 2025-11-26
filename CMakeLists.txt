cmake_minimum_required(VERSION 3.14)
project(EscapeFromTarkovInteractiveMap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(map-prototype src/main.cpp)

if(MSVC)
    target_compile_options(map-prototype PRIVATE /W4 /permissive-)
else()
    target_compile_options(map-prototype PRIVATE -Wall -Wextra -Wpedantic)
endif()
