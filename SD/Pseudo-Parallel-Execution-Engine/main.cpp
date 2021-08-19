#include <iostream>
#include <cstdio>
#include <ctime>
#include <cstdlib>

#define PRECOMPILER_FUNCTION_INITIALIZATION

#include "ppee.h"

using namespace std;
using namespace ppee;

struct State{
	int i;
	char c;
};

// MUST!
class MyFunction: public GenericFunction{
	public:
	MyFunction(){
		// initial internal state
		m_state.set(new State{rand()%10, (char)(0x41+rand()%26)});
		printf("ID[%zu] [%d, %c] init: succ\n", m_id, ((State*)m_state.get_readable())->i, ((State*)m_state.get_readable())->c);
	}

	~MyFunction(){
		// auto tmp = (State*)m_state.get_readable();
		// printf("ID[%zu] destructor called with (%d, %c)\n", m_id, tmp->i, tmp->c);
		delete (State*)m_state.get_readable();
	}

	virtual size_t run(GenericArgument& generic_argument, size_t timestamp){
		printf("ID[%zu] | Number of argument units: %zu\n", m_id, generic_argument.count());
		for(size_t i=m_id; i<=m_id; ++i){
			// get readable argument
			auto rarg = generic_argument.get_readable(i);
			if(!rarg){
				printf("Generic Argument not found!\n");
				exit(1);
			}

			// do some processing without modifiying it
			printf("arg [%zu]: %f\n", i, *(float*)rarg->arg());

			// get writable argument
			auto warg = generic_argument.get_writable(0, timestamp);
			if(!warg){
				printf("Race conditioned @ %zu!\n", i);
				return 1;
			}

			// modify it accordingly
			*(float*)(warg->arg()) = 2.f * m_id + 3.f * *(float*)(warg->arg()) + 4.f * timestamp;
		}
		return 0;
	}
};

int main(){
	// Resource creation
	float data[4] = { 0 };
	printf("Resources creation: succ\n");
	
	// Converting the resources into Generic Argument Units
	GenericArgumentUnit units[4];
	for(int i=0; i<4; ++i)
		units[i].get() = data+i;
	printf("Converting to arg units: succ\n");

	// Converting units to a Generic Argument
	GenericArgument generic_argument; 
	for(int i=0; i<4; ++i)
		generic_argument.add(units+i, i? SENSITIVE : INSENSITIVE);
	printf("Converting to generic arg: succ\n");
	
	// Generic Functions
	MyFunction funcs[4];
	vector<vector<GenericFunction*>> generic_functions =
	{
		{ funcs+0, funcs+1 },
		{ funcs+2, funcs+3 }
	};
	printf("Generic functions creation: succ\n");

	// ExecutionGraph by linking the Generic Functions
	ExecutionGraph execution_graph; 
	execution_graph.execution_style() = ExecutionStyle::CYCLIC;
	execution_graph.randomized_execution() = false;
	printf("Execution graph init: succ\n");
	execution_graph.compile(generic_functions);
	printf("Execution graph compilation: succ\n");
	
	// Engine
	Engine engine(&execution_graph, &generic_argument);
	printf("Engine init: succ\n");
	engine.iterate(6);
	printf("Terminated: succ\n");
	
	return 0;
}
