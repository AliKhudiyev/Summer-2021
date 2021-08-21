#include <cstdio>

// #define PRECOMPILER_FUNCTION_INITIALIZATION

#include "ppee.h"

using namespace std;
using namespace ppee;

class MyFunction: public GenericFunction<float>{
	public:
	MyFunction(){
		// initial internal state
	}
	~MyFunction(){}

	virtual size_t run(GenericArgument<float>& generic_argument, const Engine<float>* const engine){
		size_t ts = engine->timestamp();

		// fetch the readable argument
		const float* rarg = generic_argument.get_readable(m_id, ts);
		if(!rarg)	return -1;
		
		// do processing with the rarg
		printf("ID[%zu] TS[%zu] | rarg: %f\n", m_id, ts, *rarg);
		
		// fetch the writable argument
		float* warg = generic_argument.get_writable(m_id, ts);
		if(!warg)	return -1;

		// modify the warg
		*warg += ts * (m_id + 1);

		// logging
		engine->log(
				[](const void* const args) -> bool {
					auto engine = (Engine<float>*)args;
					return (engine->timestamp() == 1 || 
							!(engine->timestamp() % engine->opts.timestamp_steps));
				}, 
				engine, 
				"ID[%zu] TS[%zu]| Number of argument units: %zu, rarg: %.3f\n", 
				m_id, ts, generic_argument.count(), *rarg
		);
		
		return 0;
	}
};

int main(){
	using T = float;
	
	// Resources creation
	T data[4] = { 0 };
	printf("Resource creation: succ\n");

	// Converting resources to a single Generic Argument
	GenericArgument<T> generic_argument;
	for(int i=0; i<4; ++i)
		generic_argument.add(data[i], i%2 ? SENSITIVE_WRITE : INSENSITIVE);
	printf("Converting to GenericArgument: succ\n");

	// Customized Generic Functions creation
	MyFunction funcs[4];
	printf("GenericFunction creation: succ\n");

	// Execution Graph creation & adding Generic Functions
	ExecutionGraph<T> execution_graph;
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
	Engine<T> engine(&execution_graph, &generic_argument);
	engine.opts.verbosity = VERBOSITY_ALL;
	engine.opts.timestamp_steps = 3;
	engine.log_file_path("test.log");
	printf("Engine init: succ\n");
	engine.iterate(8);
	printf("Terminated: succ\n");

	return 0;
}
