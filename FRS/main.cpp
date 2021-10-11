#include <cstdio>

#include "ppee.h"

using namespace std;
using namespace ppee;

struct CoreState;

using T = CoreState;
// given the input cores which are activated the list of outputted activations
using pattern_t = pair<vector<const GenericFunction<T>* const> /* inputs */, const GenericFuction<T>* const /* outputs */>


struct Position{
	float coords[3];

	inline float& x() { return coords[0]; }
	inline float& y() { return coords[1]; }
	inline float& z() { return coords[2]; }
};

struct CoreState{
	// statically defined params
	Position position;

	// dynamically defined params
	vector<pattern_t> patterns;
};

struct CoreGenOpts{

};

struct InterconnectTable{
	vector<vector<bool>> table;

	bool connectivity_between(size_t core_id1, size_t core_id2){
		;
	}

	template<typename T>
	bool connectivity_between(const GenericFuction<T>* const generic_function1, const GenericFunction<T>* const generic_function2){
		;
	}
};

class Core: public GenericFunction<T>{
	static InterconnectTable interconnect_table;

	public:
	Core(){
		// ...
	}
	virtual ~Core(){}

	virtual size_t run(GenericArgument<T>& generic_argument, const Engine<T>* const engine){
		// ...
	}

	static vector<Core*> generate(size_t n, const CoreGenOpts& opts){
		// TODO
	}
	
	bool is_connected_to(size_t id){
		return Core::interconnect_table.connectivity_between(id(), id);
	}

	bool is_connected_to(const GenericFunction<T>* const generic_function){
		return Core::interconnect_table.connectivity_between(this, generic_function);
	}
};

int main(){
	;
	return 0;
}
