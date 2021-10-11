#include "memory.h"

namespace alc{ namespace memory{
	void Memory::push(bool bit, size_t index){
	}

	void Memory::push_set(){
	}

	BitStream Memory::operator[](size_t index){
		return m_data[index];
	}
} }
