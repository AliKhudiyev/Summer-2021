CXX = g++
CXX_FLAGS = -std=c++11
LINKER_FLAG = -lcppunit
BUILD_DIR = build


all: ${BUILD_DIR}/bit_stream ${BUILD_DIR}/tests

${BUILD_DIR}/bit_stream: bit_stream.cpp bit_stream.h
	${CXX} ${CXX_FLAGS} bit_stream.cpp  -o $@

${BUILD_DIR}/tests: tests.cpp
	${CXX} ${CXX_FLAGS} ${LINKER_FLAG} $^ -o $@

check_leaks: ${BUILD_DIR}/bit_stream ${BUILD_DIR}/tests
	leaks -atExit -- ${BUILD_DIR}/bit_stream
	leaks -atExit -- ${BUILD_DIR}/tests
