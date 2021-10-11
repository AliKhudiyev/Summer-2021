
#pragma once

#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <cstddef>
#include <random>
#include <memory>
#include <functional>

#include "bit_stream.h"
#include "ppee.h"
#include "memory.h"

#define CORE_MIN_MEMORY_SIZE 2		// 2 b
#define CORE_MAX_MEMORY_SIZE 1024	// 128 B

#define INTERCONNECT_NONSPECULATIVE true
#define INTERCONNECT_SPECULATIVE 	false

namespace alc{ namespace core{
	using namespace ppee;
	using namespace memory;

	using policy_t = bool;
	using stm_t = BitStream;

	enum CoreType{
		DEFAULT = 0,
		SELECTOR_0,
		SELECTOR_1,
		NEGATOR
	};

	class Core;
	struct InterconnectInfo{
		float support;
		bool speculative;

		InterconnectInfo(): support(0.f), speculative(false) {}
		InterconnectInfo(float support_, bool speculative_): 
			support(support_), speculative(speculative_) {}
		InterconnectInfo(float support_): support(support_), speculative(false) {}
		InterconnectInfo(bool speculative_): support(0.f), speculative(speculative_) {}
		~InterconnectInfo() = default;
	};

	struct Interconnect{
		InterconnectInfo info;
		std::pair<std::weak_ptr<Core>, std::weak_ptr<Core>> cores;

		inline std::shared_ptr<Core> from() const{ return cores.first.lock(); }
		inline std::shared_ptr<Core> to() const{ return cores.second.lock(); }
		inline bool is_speculative() const{ return info.speculative; }
		inline void convert_to(bool nonspeculative){ info.speculative = !nonspeculative; }

		bool operator==(const Interconnect& interconnect) const{
			return (cores.first.lock() == interconnect.cores.first.lock() &&
				cores.second.lock() == interconnect.cores.second.lock()) ||
				(cores.first.lock() == interconnect.cores.second.lock() &&
				cores.second.lock() == interconnect.cores.first.lock() && 
				info.speculative && interconnect.info.speculative);
		}
		bool operator!=(const Interconnect& interconnect) const{
			return !(*this == interconnect);
		}

		// returns the interconnect metadata
		static InterconnectInfo get_info(const std::shared_ptr<Core>& core1, 
				const std::shared_ptr<Core>& core2);
	};

	class Core: public std::enable_shared_from_this<Core>, public GenericFunction<bool>{
		public:
		using interconnect_t = std::pair<std::weak_ptr<Core>, InterconnectInfo>;

		private:
		static size_t core_count;
		static std::hash<std::string> hash_string;

		public:
		// unique id for each instance
		size_t m_id;
		// hash values according to backward connections
		size_t m_hash;
		// number of interconnects from the input level
		size_t m_depth;
		// core types
		CoreType m_type;
		// core name
		std::string m_name;
		// core [final] output value
		bool m_output;
		// input values
		BitStream m_inputs;
		// directed interconnects; flow direction: backward -> this -> forward
		// std::vector<std::weak_ptr<Core>> m_backward, m_forward;
		std::vector<interconnect_t> m_backward, m_forward;
		// internal short term memory
		mutable stm_t m_memory;

		public:
		Core(size_t memory_size=4, bool create_input=false):
			m_id(++core_count), m_depth(0), m_type(CoreType::DEFAULT), m_output(0)
		{
			if(create_input)
				m_inputs.reset(1, 0);
			if(memory_size > CORE_MAX_MEMORY_SIZE)
				memory_size = CORE_MAX_MEMORY_SIZE;
			else if(memory_size < CORE_MIN_MEMORY_SIZE)
				memory_size = CORE_MIN_MEMORY_SIZE;
			m_memory.reset(memory_size);
		}
		virtual ~Core() = default;

		inline virtual CoreType type() const{ return m_type; }
		virtual void set_name(const std::string& name);
		// inline virtual std::string& name(){ return m_name; }
		inline virtual const std::string& name() const{ return m_name; }
		inline virtual size_t memory_size() const{ return m_memory.size(); }
		virtual void set_memory(const stm_t& memory);
		virtual void reset_memory(bool bit=0);
		virtual float get_memory_usage() const;
		inline virtual const stm_t& get_memory() const{ return m_memory; }

		virtual size_t id() const;
		virtual size_t hash() const;
		virtual size_t depth() const;
		virtual const std::vector<interconnect_t>& backward_connections() const;
		virtual const std::vector<interconnect_t>& forward_connections() const;
		virtual const InterconnectInfo& interconnect_info(const std::shared_ptr<Core>& core) const;
		virtual bool connected(const std::shared_ptr<Core>& core) const;

		// a.connect(b) means a -> b
		virtual bool connect(const std::shared_ptr<Core>& core);
		// a.disconnect(b) means a b
		virtual bool disconnect(const std::shared_ptr<Core>& core);

		// force set the [final] output value
		virtual void set(bool bit);
		// returns the [final] output value
		virtual bool get() const;
		// returns the [final] output value by propagating from the root cores, does not overwrite
		virtual bool pull() const;
		// returns the [final] output value by propagating from the root cores, overwrites output+memory
		virtual bool propagate_get();
		// returns the [final] output and then propagates
		virtual bool propagate(bool stm=true);
		// check if the cores have the same backward connections
		virtual bool replaceable_with(const std::shared_ptr<Core>& core) const;
		// replaces the current core with the given one
		virtual bool replace_with(std::shared_ptr<Core> core);
		// removes the forward connections of the core if not connected (core.m_forward.empty())
		virtual void remove_if_not_connected();

		size_t run(GenericArgument<bool>& argument, const Engine<bool>* const engine){
			return 0;
		}

		protected:
		// sets and shift pushes into the memory if needed
		virtual void set(bool bit, bool stm);
		// accepts connection coming from the `core` (like doubly linked list)
		virtual bool accept_connection(const std::shared_ptr<Core>& core);
		// refuses connection coming from the `core` (for disconnecting)
		virtual bool refuse_connection(const std::shared_ptr<Core>& core);
		// updates list of incoming signals
		virtual void receive(bool bit, const std::shared_ptr<Core>& core);
		// updates depth information, backward+forward hashes
		virtual void update();
		// returns the [final] output without overwriting
		virtual bool get(const BitStream& inputs) const;
	};

	template<policy_t BIT>
	class Selector: public Core{
		public:
		Selector(size_t memory_size=4): Core(memory_size)
		{
			m_type = static_cast<CoreType>(CoreType::SELECTOR_0+BIT);
			m_inputs.reset(0);
		}
		~Selector() = default;

		protected:
		bool get(const BitStream& inputs) const{
			for(size_t i=0; i<inputs.size(); ++i){
				if(inputs[i] == BIT)
					return BIT;
			}
			return !BIT;
		}
	};

	class NOT: public Core{
		public:
		NOT(size_t memory_size=4): Core(memory_size)
		{
			m_type = CoreType::NEGATOR;
			
		}
		~NOT() = default;

		protected:
		bool get(const BitStream& inputs) const{
			return !inputs[0];
		}
	};

	// Optional synonyms
	using AND = Selector<0>;
	using OR = Selector<1>;
	using AND_1 = AND;
	using AND_0 = OR;
	using OR_1 = OR;
	using OR_0 = AND;
} }

