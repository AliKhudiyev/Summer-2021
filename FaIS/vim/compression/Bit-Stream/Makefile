CXX = g++
CXX_FLAGS = -std=c++11
LINKER_FLAG = -lcppunit
BUILD_DIR = build


all: bit_stream tests

bit_stream: bit_stream.cpp
	${CXX} ${CXX_FLAGS} $^ -o ${BUILD_DIR}/$@

tests: tests.cpp
	${CXX} ${CXX_FLAGS} ${LINKER_FLAG} $^ -o ${BUILD_DIR}/$@

run_tests: tests
	${BUILD_DIR}/$^
