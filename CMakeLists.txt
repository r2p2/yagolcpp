cmake_minimum_required(VERSION 3.0)
project(yagolcpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib 4.2.0 REQUIRED)

add_executable(yagolcpp
	src/main.cc
)
target_link_libraries(yagolcpp
	PRIVATE raylib
)
