
#pragma once

#include <vector>
#include <string>
#include <map>
#include <cstdio>

namespace ppee{
	/*
	 * Time-sensitive Generic Argument Unit class
	 * This class is to initialize a piece of data which is sensitive to time.
	 * It means, this type of data is not open to race conditions.
	 * If two writes occur simultaneously, error(s) or warning(s) may be thrown.
	 */
	class GenericArgumentUnit{
		private:
		// pointer to the unit of arguments; should be cast to a non-void pointer by the user
		void* m_args;
		// timestamp of the most recently occured change on this data unit
		size_t m_timestamp;

		public:
		GenericArgumentUnit(): m_args(nullptr), m_timestamp(0) {}
		GenericArgumentUnit(void* args): m_args(args), m_timestamp(0) {}
		~GenericArgumentUnit(){}

		inline void*& get() { return m_args; }
		inline void*& arg() { return m_args; }
		inline size_t& timestamp() { return m_timestamp; }
	};

	/*
	 * Generic Argument class
	 */
	class GenericArgument{
		private:
		std::vector<GenericArgumentUnit*> m_args;

		public:
		bool is_race_conditioned(size_t index, size_t timestamp) {
			if(m_args[index]->timestamp() == timestamp)
				return true;
			m_args[index]->timestamp() = timestamp;
			return false;
		}

		inline void add(GenericArgumentUnit* argument_unit) { m_args.push_back(argument_unit); }
		inline void remove(size_t index=-1 /* last element */) { m_args.pop_back(); }
		inline void clear() { m_args.clear(); }

		inline size_t count() const { return m_args.size(); }
		inline GenericArgumentUnit get(size_t index, size_t timestamp=-1 /* doesn't matter */) const { 
			// Not handling index out of range exception
			return *m_args[index]; 
		}
		inline GenericArgumentUnit* set(size_t index, size_t timestamp) { 
			if(m_args[index]->timestamp() == timestamp){
				// Race condition handler
				return nullptr;
			}
			if(index >= m_args.size()){
				// Index out of range handler
				return nullptr;
			}
			m_args[index]->timestamp() = timestamp;
			return m_args[index];
		}
		
		// same as get(size_t) function
		inline GenericArgumentUnit get_readable(size_t index, size_t timestamp=-1 /* doesn't matter */) const { return get(index, timestamp); }
		// same as set(size_t, size_t) function
		inline GenericArgumentUnit* get_writable(size_t index, size_t timestamp) { return set(index, timestamp); }
	};
	
	/*
	 * Generic Function class
	 * Utilizes the generic argument in which changes can be made.
	 * 'run' virt. method has to be defined by the user since this function is called 
	 * through this function by the PPEE.
	 */
	class GenericFunction{
		static size_t generic_function_count;
		static std::map<std::string /* name */, size_t /* id */> function_reservations;

		protected:
		size_t m_id;
		std::string m_name;

		public:
		GenericFunction(const std::string& name=""):
			m_id(GenericFunction::generic_function_count++),
			m_name(name)
		{
			// ...
		}

		// This function has to be implemented explicityly by the user!
		// Returns the number of modified generic argument units (optional)
		virtual size_t run(GenericArgument& generic_argument, size_t timestamp) = 0;
	};

	size_t GenericFunction::generic_function_count = 0;
	std::map<std::string, size_t> GenericFunction::function_reservations = std::map<std::string, size_t>();
	
	enum class ExecutionStyle = {
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

		public:
		void add(GenericFunction* function, size_t timestamp=-1){
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

		void compile(){
			m_generic_functions.clear();
			for(auto it=m_function_timestamps.begin(); it!=m_function_timestamps.end(); ++it){
				m_generic_functions.push_back(it->second);
			}
		}
		
		void compile(const std::vector<std::vector<GenericFunction*>> generic_functions){
			m_generic_functions = generic_functions;
		}

		std::vector<GenericFunction*>& get_curr(int direction=1){
			auto& generic_function_vec = m_generic_functions[m_index];
			if(m_execution_style == ExecutionStyle::CYCLIC || 
			  (m_execution_style == ExecutionStyle::DEFAULT && m_index+direction < m_generic_functions.size())){
				m_index = (m_index + direction) % m_generic_functions.size();
			}
			return generic_function_vec;
		}

		std::vector<GenericFunction*>& get(size_t index){
			if(index >= m_generic_functions.size()){
				// TODO: has to be handled
			}
			m_index = index;
			return m_generic_functions[index];
		}

		std::vector<GenericFunction*>& get_prev(){
			size_t tmp = m_index;
			m_index = (m_index-1) % m_generic_functions.size();
			if(m_execution_style == ExecutionStyle::DEFAULT && !tmp){
				// TODO: has to be handled
			}
			return get((tmp-1) % m_generic_functions.size());
		}

		std::vector<GenericFunction*>& get_next(){
			size_t tmp = m_index;
			m_index = (m_index+1) % m_generic_functions.size();
			if(m_execution_style == ExecutionStyle::DEFAULT && tmp+1 == m_generic_functions.size()){
				// TODO: has to be handled
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
				auto& generic_functions = m_execution_graph->get_curr();	
				for(size_t i=0; i<generic_functions.size(); ++i){
					generic_functions[i]->run(*m_generic_argument, m_timestamp);
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

