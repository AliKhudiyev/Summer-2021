
#pragma once

#include "core.h"
#include "memory.h"
#include "defs.h"
#include "parameters.h"
#include <random>

using namespace alc::core;
using namespace alc::memory;
using namespace alc::parameters;

#define SYSTEM_MIN_MEMORY_SIZE 1e6		// 125 KB
#define SYSTEM_MAX_MEMORY_SIZE 32*1e9 	// 4 GB

namespace alc{
	class IO;

	class System{
		friend class IO;

		enum CoreSubtype{
			INPUT = 5,			// 00101
			OUTPUT = 6,			// 00110
			INPUT_OUTPUT = 7,	// 00111
			SELECTOR_0 = 9,		// 01001
			SELECTOR_1 = 10,	// 01010
			SELECTOR = 11,		// 01011
			NEGATOR = 12,		// 01100
			FUNCTIONAL = 15,	// 01111
			ALL = 31			// 11111
		};

		enum InterconnectSubtype{
			REGULAR = 0,
			SPECULATIVE = 1
		};

		private:
		Options m_options;
		Policy m_policy;
		Stats m_stats;
		size_t m_input_count, m_output_count;
		size_t m_iteration_count;
		BitStream m_specialized_for;
		std::vector<Interconnect> m_interconnects;

		std::vector<std::shared_ptr<Core>> m_inputs;
		std::vector<std::shared_ptr<Core>> m_outputs;
		std::vector<std::shared_ptr<Core>> m_cores;
		Memory m_memory;
		
		public:
		System(size_t n_input, size_t n_output, 
				const Policy& policy, const Options& options): 
			m_iteration_count(0), m_policy(policy), m_options(options)
		{
			if(n_input < 1 || n_output < 1){
				printf("Invalid input count [%zu] or output count [%zu]\n", n_input, n_output);
				exit(1);
			}

			auto& core_memory_size = m_policy.core_memory_size;
			auto& system_memory_size = m_policy.system_memory_size;

			if(system_memory_size > SYSTEM_MAX_MEMORY_SIZE)
				system_memory_size = SYSTEM_MAX_MEMORY_SIZE;
			else if(system_memory_size < SYSTEM_MIN_MEMORY_SIZE)
				system_memory_size = SYSTEM_MIN_MEMORY_SIZE;

			if(core_memory_size > CORE_MAX_MEMORY_SIZE)
				core_memory_size = CORE_MAX_MEMORY_SIZE;
			else if(core_memory_size < CORE_MIN_MEMORY_SIZE)
				core_memory_size = CORE_MIN_MEMORY_SIZE;

			m_inputs.resize(n_input);
			m_outputs.resize(n_output);
			
			for(size_t i=0; i<n_input; ++i){
				auto core = std::make_shared<Core>(core_memory_size, true);
				core->set_name("i_" + std::to_string(i));
				m_inputs[i] = core;
			}
			for(size_t i=0; i<n_output; ++i){
				auto core = std::make_shared<Core>(core_memory_size);
				core->set_name("o_" + std::to_string(i));
				m_outputs[i] = core;
			}
		}
		System(const char* system_path, const char* options_path, const Options& options):
			m_options(options)
		{
			if(load(system_path, options_path) != LOAD_OK){
				printf("Invalid system path [%s] or options path [%s]\n", system_path, options_path);
				exit(1);
			}
		}
		~System() = default;

		void reset(const Policy& policy, const Options& options);
		void set_policy(const Policy& policy);
		void set_options(const Options& options);
		const Policy& get_policy() const;
		const Options& get_options() const;
		const Stats& get_stats() const;

		void fit(IO& io);
		raw_output_t fit_predict(const raw_input_t& input, const raw_output_t& output);
		void fit(const raw_input_t& input, const raw_output_t& output);

		void predict(const raw_input_t& input, raw_output_t& output);
		raw_output_t predict(const raw_input_t& input);
		void predict(IO& io);

		/* --- System ---
		 * id, type, subtype, id1, id2, depth, support
		 * type: core/interconnect
		 * subtype: input, output, selector_0, selector_1, not; regular, speculative
		 * id1 -> id2 (if regular), id1 - id2 (if speculative)
		 *
		 * --- Options + Policy ---
		 * [key] [value]
		 * system_path out.sys
		 * -
		 * policy 0
		 */
		void save(const char* sys_path, const char* opts_path, const char* mode="w");
		bool load(const char* sys_path, const char* opts_path);

		std::vector<std::string> to_strings() const;
		void from_strings(const std::vector<std::string>& expr);
		float get_memory_usage() const;
		inline size_t get_iteration_count() const{ return m_iteration_count; }

		private:
		System(size_t core_memory_size=16, size_t system_memory_size=1e6):
			m_iteration_count(0)
		{
			if(system_memory_size > SYSTEM_MAX_MEMORY_SIZE)
				system_memory_size = SYSTEM_MAX_MEMORY_SIZE;
			else if(system_memory_size < SYSTEM_MIN_MEMORY_SIZE)
				system_memory_size = SYSTEM_MIN_MEMORY_SIZE;

			if(core_memory_size > CORE_MAX_MEMORY_SIZE)
				core_memory_size = CORE_MAX_MEMORY_SIZE;
			else if(core_memory_size < CORE_MIN_MEMORY_SIZE)
				core_memory_size = CORE_MIN_MEMORY_SIZE;

			// TODO
		}

		std::shared_ptr<Core> find_core_by_id(size_t id, CoreSubtype subtype);
		std::shared_ptr<Core> find_core_by_hash(size_t hash, CoreSubtype subtype);
		std::shared_ptr<Core> find_core_by_name(const std::string& name, CoreSubtype subtype);
		Interconnect find_interconnect(size_t id1, size_t id2, InterconnectSubtype subtype);
		std::vector<std::shared_ptr<Core>> all_cores() const;
		bool make_interconnect(const std::shared_ptr<Core>& core1, const std::shared_ptr<Core>& core2);
		size_t max_depth() const;
		const std::vector<std::shared_ptr<Core>> get(size_t depth) const;
		void propagate(size_t depth) const;
		void run_diagnostics();
		void update_interconnects();
		const std::vector<Interconnect>& get_interconnects() const;
		Interconnect* get_interconnect(const std::weak_ptr<Core>& core1, 
				const std::weak_ptr<Core>& core2);
		void stabilize(bool stm=true);
		System create(const raw_input_t& input, const raw_output_t& output) const;
		void compress(const System& system);
		static void to_string(std::string& expr, const std::weak_ptr<Core>& core_);
		void from_string(const std::string& expr);
	};
}

