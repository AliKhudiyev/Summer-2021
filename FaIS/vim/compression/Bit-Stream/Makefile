CXX = g++
CXX_FLAGS = -std=c++11
LINKER_FLAG = -lcppunit

all: bit_stream tests

bit_stream: bit_stream.cpp
	${CXX} ${CXX_FLAGS} $^ -o $@

tests: tests.cpp
	${CXX} ${CXX_FLAGS} ${LINKER_FLAG} $^ -o $@
