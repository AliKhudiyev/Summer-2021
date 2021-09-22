CXX = g++
CXX_FLAGS = -std=c++11
LINKER_FLAG = -lcppunit
BUILD_DIR = build


all: ${BUILD_DIR}/main ${BUILD_DIR}/tests

${BUILD_DIR}/main: main.cpp bit_stream.h
	${CXX} ${CXX_FLAGS} main.cpp  -o $@

${BUILD_DIR}/tests: tests.cpp
	${CXX} ${CXX_FLAGS} ${LINKER_FLAG} $^ -o $@

check_leaks: ${BUILD_DIR}/main ${BUILD_DIR}/tests
	leaks -atExit -- ${BUILD_DIR}/bit_stream
	leaks -atExit -- ${BUILD_DIR}/tests
