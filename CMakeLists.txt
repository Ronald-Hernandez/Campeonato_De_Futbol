cmake_minimum_required(VERSION 3.19)
project(CampeonatoFutbolConsola LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CampeonatoFutbolConsola
    main.cpp
)

target_link_libraries(CampeonatoFutbolConsola PRIVATE odbc32)
