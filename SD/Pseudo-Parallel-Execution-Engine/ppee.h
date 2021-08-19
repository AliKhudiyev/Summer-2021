
#pragma once

#include <vector>
#include <string>
#include <map>
#include <cstdio>
#include <algorithm>
#include <random>

#define SENSITIVE	true
#define INSENSITIVE	false

#define ALLOC_TYPE_STACK 	true
#define ALLOC_TYPE_HEAP		false

namespace ppee{
	/*
	 * Time-sensitive/insensitive Generic Argument Unit class
	 * This class is to initialize a piece of data which is sensitive to time.
	 * It means, this type of data is not open to race conditions.
	 * If two writes occur simultaneously, error(s) or warning(s) may be thrown.
	 *
	 * Time-sensitive units are not allowed to become race conditioned.
	 * Two "processes" can read but not write to the same unit simultaneously.
	 *
	 * Time-insensitive units are allowed to become race conditioned and
	 * such race conditions are handled so that the program does not malbehave.
	 * Two "processes" can read and/or write to the same unit simutaneously, 
	 * and each of such "processes" is going to execute in an ordered manner.
	 */
	class GenericArgumentUnit{
		private:
		// pointer to the unit of arguments; should be cast to a non-void pointer by the user
		void* m_args;
		// timestamp of the most recently occured change on this data unit
		size_t m_timestamp;
		// time sensitivity: false - insensitive, true - sensitive
		bool m_sensitivity;

		public:
		GenericArgumentUnit(void* args=nullptr, bool sensitivity=false): m_args(args), m_timestamp(0), m_sensitivity(sensitivity) {}
		~GenericArgumentUnit(){}

		inline void*& get() { return m_args; }
		inline void* get() const { return m_args; }
		inline void*& arg() { return m_args; }
		inline void* arg() const { return m_args; }
		inline size_t& timestamp() { return m_timestamp; }
		inline const size_t& timestamp() const { return m_timestamp; }
		inline bool& sensitivity() { return m_sensitivity; }
		inline const bool& sensitivity() const { return m_sensitivity; }
	};

	/*
	 * Generic Argument class
	 */
	class GenericArgument{
		using alloc_t = bool;

		private:
		std::vector<GenericArgumentUnit*> m_args;
		GenericArgumentUnit m_args_unit;
		alloc_t m_alloc_type;

		public:
		GenericArgument(alloc_t alloc_type=ALLOC_TYPE_STACK): m_alloc_type(alloc_type) {}
		~GenericArgument(){
			if(m_alloc_type == ALLOC_TYPE_HEAP){
				for(auto& arg: m_args)
					delete arg;
			}
		}

		inline void add(GenericArgumentUnit* argument_unit, bool sensitivity=INSENSITIVE) { 
			if(m_alloc_type == ALLOC_TYPE_HEAP){
				printf("[WARNING]: GenericArgumentUnit* couldn't be added due to (stack) allocation ambiguity!\n");
				return ;
			}
			argument_unit->sensitivity() = sensitivity;
			m_args.push_back(argument_unit); 
		}
		inline void add(void* args, bool sensitivity=INSENSITIVE) {
			if(m_alloc_type == ALLOC_TYPE_STACK){
				printf("[WARNING]: GenericArgumentUnit couldn't be added due to (heap) allocation ambiguity!\n");
				return ;
			}
			else if(!args){
				printf("[WARNING]: GenericArgumentUnit couldn't be added due to NULL args!\n");
				return ;
			}
			m_args.push_back(new GenericArgumentUnit(args, sensitivity));
		}
		inline bool remove(size_t index=-1 /* last element */) { 
			if(index == -1)	m_args.pop_back(); 
			else if(index < m_args.size()) 	m_args.erase(m_args.begin()+index);
			else 	return false;
			return true;
		}
		inline void clear() { m_args.clear(); }

		/* API for GAaGS (Generic Argument as Generic State) */
		// sets the generic state
		inline void set(void* args, bool sensitivity=INSENSITIVE) {
			m_args_unit.arg() = args;
			m_args.clear();
			m_args.push_back(&m_args_unit);
		}
		// unsets the generic state
		inline void unset() { m_args_unit.arg() = nullptr; }
		/* - - - - - - - - - - - - - - - - - - - - - - - - - */

		inline size_t count() const { return m_args.size(); }
		// returns a readable argument unit which does not violate time-(in)sensitivy
		// timestamp - (optional) used for debugging purposes
		inline const GenericArgumentUnit* get_readable(size_t index, size_t timestamp=-1 /* optional */) const { 
			if(index >= m_args.size())
				// Index out of range handler
				return nullptr;
			if(timestamp != -1) m_args[index]->timestamp() = timestamp;
			return m_args[index]; 
		}
		
		// 'get_readable' function for Generic State
		inline const void* get_readable() const { 
			return get_readable(0)->arg();
		}

		// returns a writable argument unit which may violate time-sensitivity (if set)
		// if time-sensitive unit is race-conditioned then returns nullptr, otherwise a pointer to the unit
		inline GenericArgumentUnit* get_writable(size_t index, size_t timestamp) { 
			if(index >= m_args.size()){
				// Index out of range handler
				return nullptr;
			}
			if(m_args[index]->sensitivity() && m_args[index]->timestamp() == timestamp){
				// Race condition handler
				return nullptr;
			}
			m_args[index]->timestamp() = timestamp;
			return m_args[index];
		}
		// 'get_writable' function for Generic State
		inline void* get_writable(size_t timestamp) { 
			return get_writable(0, timestamp)->arg();
		}
		// 'get_writable' function for time-insensitive Generic State
		inline void* get_writable() {
			auto tmp = get_writable(0, 0);
			if(tmp->sensitivity() == SENSITIVE){
				// Access denied
				return nullptr;
			}
			return tmp->arg();
		}
	};
	
	/*
	 * Generic Function class
	 * Utilizes the generic argument in which changes can be made.
	 * 'run' virt. method has to be defined by the user since this function is called 
	 * through this function by the PPEE.
	 */
	class GenericFunction{
		static size_t generic_function_count;
		static std::map<std::string /* name */, size_t /* id */> function_reservations; // NOT USED

		protected:
		size_t m_id;
		std::string m_name;
		GenericArgument m_state;

		public:
		GenericFunction(const std::string& name=""):
			m_id(GenericFunction::generic_function_count++),
			m_name(name)
		{
			// ...
		}
		virtual ~GenericFunction(){}

		// This function has to be implemeted explicitly by the user!
		// It is used to initialize the internal state of the function (recommended if internal state matters)
		virtual void init(){} /* = 0; */
		virtual void tear(){} /* = 0; */

		// This function has to be implemented explicitly by the user!
		// Returns the number of modified generic argument units (optional)
		virtual size_t run(GenericArgument& generic_argument, size_t timestamp) = 0;
	};

	size_t GenericFunction::generic_function_count = 0;
	std::map<std::string, size_t> GenericFunction::function_reservations = std::map<std::string, size_t>();
	
	enum class ExecutionStyle {
		DEFAULT = 0,
		CYCLIC
	};

	/*
	 * API for Execution Path
	 * Execution Path defines the timestamps in which generic functions are going
	 * to be executed.
	 */
	class ExecutionGraph{
		private:
		std::map<size_t /* timestamp */, std::vector<GenericFunction*> /* functions */> m_function_timestamps;
		std::vector<std::vector<GenericFunction*>> m_generic_functions;
		// ExecutionGraph *prev, *next;
		size_t m_index = 0, m_min_timestamp = 0, m_max_timestamp = 0;
		ExecutionStyle m_execution_style = ExecutionStyle::DEFAULT;
		bool m_randomized_execution = false;

		public:
		void add(GenericFunction* function, size_t timestamp=-1){
#ifdef PRECOMPILER_FUNCTION_INITIALIZATION
			function->init();
#endif
			if(timestamp == -1){
				m_function_timestamps[++m_max_timestamp].push_back(function);
			} else{
				m_function_timestamps[timestamp].push_back(function);
				if(timestamp > m_max_timestamp)	m_max_timestamp = timestamp;
			}	
		}

		void clear(){
			m_function_timestamps.clear();
		}

		void compile(bool randomize=false){
			m_generic_functions.clear();
			for(auto it=m_function_timestamps.begin(); it!=m_function_timestamps.end(); ++it){
				if(randomize)	std::random_shuffle(it->second.begin(), it->second.end());
#ifndef PRECOMPILER_FUNCTION_INITIALIZATION
				for(auto it2=it->second.begin(); it2!=it->second.end(); ++it2)
					(*it2)->init();
#endif
				m_generic_functions.push_back(it->second);
			}
			
		}
		
		void compile(const std::vector<std::vector<GenericFunction*>> generic_functions, bool randomize=false){
			m_generic_functions = generic_functions;
			for(auto it=m_generic_functions.begin(); it!=m_generic_functions.end(); ++it){
				if(randomize)
					std::random_shuffle(it->begin(), it->end());
				for(auto it2=it->begin(); it2!=it->end(); ++it2)
					(*it2)->init();
			}
		}

		inline ExecutionStyle& execution_style() { return m_execution_style; }
		inline bool& randomized_execution() { return m_randomized_execution; }

		std::vector<GenericFunction*>* get_curr(int direction=1){
			if(m_index == -1) return nullptr;

			if(m_randomized_execution)
				std::random_shuffle(m_generic_functions[m_index].begin(), m_generic_functions[m_index].end());	

			auto generic_function_vec = &m_generic_functions[m_index];
			if(m_execution_style == ExecutionStyle::CYCLIC || 
			  (m_execution_style == ExecutionStyle::DEFAULT && m_index+direction < m_generic_functions.size())){
				m_index = (m_index + direction) % m_generic_functions.size();
			} else m_index = -1;
			return generic_function_vec;
		}

		std::vector<GenericFunction*>* get(size_t index){
			if(index >= m_generic_functions.size()){
				// Index out of range handling
				return nullptr;
			}
			m_index = index;
			if(m_randomized_execution)
				std::random_shuffle(m_generic_functions[index].begin(), m_generic_functions[index].end());
			return &m_generic_functions[index];
		}

		std::vector<GenericFunction*>* get_prev(){
			size_t tmp = m_index;
			m_index = (m_index-1) % m_generic_functions.size();
			if(m_execution_style == ExecutionStyle::DEFAULT && !tmp){
				return nullptr;
			}
			return get((tmp-1) % m_generic_functions.size());
		}

		std::vector<GenericFunction*>* get_next(){
			size_t tmp = m_index;
			m_index = (m_index+1) % m_generic_functions.size();
			if(m_execution_style == ExecutionStyle::DEFAULT && tmp+1 == m_generic_functions.size()){
				return nullptr;
			}
			return get((tmp+1) % m_generic_functions.size());
		}
	};

	class Engine{
		private:
		ExecutionGraph* m_execution_graph;
		GenericArgument* m_generic_argument;
		size_t m_timestamp;

		public:
		Engine(ExecutionGraph* execution_graph, GenericArgument* generic_argument):
			m_execution_graph(execution_graph),
			m_generic_argument(generic_argument),
			m_timestamp(0)
		{
			// ...
		}

		// runs for n iterations
		int iterate(size_t n=1){
			while(n--){
				++m_timestamp;
				printf(" > Timestamp: %zu\n", m_timestamp);

				auto generic_functions = m_execution_graph->get_curr();	
				if(!generic_functions)	return n;

				for(size_t i=0; i<generic_functions->size(); ++i){
					(*generic_functions)[i]->run(*m_generic_argument, m_timestamp);
				}
			}
			return n;
		}
		// iterates till an error occurs
		int iterate_all(){
			// TODO
			return 0;
		}
		// runs till the end of execution; may not halt for a long time
		int run(){
			// TODO
			return 0;
		}
	};
}

