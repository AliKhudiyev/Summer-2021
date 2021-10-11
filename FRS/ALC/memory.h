
#pragma once

#include <vector>
#include "bit_stream.h"

namespace alc{ namespace memory{
	class Memory{
		private:
		size_t m_max_set_count;
		size_t m_max_unit_count;
		std::vector<BitStream> m_data;

		public:
		void push(bool bit, size_t index);
		void push_set();
		virtual float get_memory_usage() const{ return 0.f; }

		BitStream operator[](size_t index);
	};

	class TemporalMemory: public Memory{
		private:

		public:

	};
} }

