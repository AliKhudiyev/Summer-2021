cmake_minimum_required(VERSION 3.10)

project(ALC)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(main main.cpp
	system.cpp
	core.cpp
	memory.cpp
	io.cpp
	utils.cpp
)

add_executable(alc alc.cpp
	system.cpp
	core.cpp
	memory.cpp
	io.cpp
	utils.cpp
)
