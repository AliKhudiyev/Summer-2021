#include <cstdio>

#define PRECOMPILER_FUNCTION_INITIALIZATION

#include "ppee.h"

using namespace std;
using namespace ppee;

class MyFunction: public GenericFunction{
	public:
	MyFunction(){
		// initial internal state
	}
	~MyFunction(){}

	virtual size_t run(GenericArgument& generic_argument, size_t timestamp){
		// ...
		printf("ID[%zu] | Number of argument units: %zu\n", m_id, generic_argument.count());
		
		return 0;
	}
};

int main(){
	// Resources creation
	float data[4] = { 0 };
	printf("Resource creation: succ\n");

	// Converting resources to a single Generic Argument
	// This Generic Argument stores heap-allocated elements
	GenericArgument generic_argument(ALLOC_TYPE_HEAP);
	for(int i=0; i<4; ++i)
		generic_argument.add(data+i, i%2 ? SENSITIVE : INSENSITIVE);
	printf("Converting to GenericArgument: succ\n");

	// Customized Generic Functions creation
	MyFunction funcs[4];
	printf("GenericFunction creation: succ\n");

	// Execution Graph creation & adding Generic Functions
	ExecutionGraph execution_graph;
	execution_graph.execution_style() = ExecutionStyle::CYCLIC;
	execution_graph.randomized_execution() = false;
	for(int i=0; i<4; ++i){
		size_t timestamp = rand() % 2;
		printf("%dth timestamp: %zu\n", i, timestamp);
		execution_graph.add(funcs+i, timestamp);
	}
	printf("ExecutionGraph init: succ\n");
	execution_graph.compile();
	printf("ExecutionGraph complication: succ\n");

	// Engine
	Engine engine(&execution_graph, &generic_argument);
	printf("Engine init: succ\n");
	engine.iterate(8);
	printf("Terminated: succ\n");

	return 0;
}
