
#pragma once

#include <vector>
#include <string>
#include <map>
#include <cstdio>
#include <cstdarg>
#include <algorithm>
#include <random>
#include <cmath>


#define INSENSITIVE 	0
#define SENSITIVE_WRITE	1
#define SENSITIVE_READ	2
#define SENSITIVE_ALL	3

// #define VERBOSITY_NONE 		0
#define VERBOSITY_DEFAULT 	1
#define VERBOSITY_NORMAL	2
#define VERBOSITY_ALL		3

namespace ppee{
	template<class T>
	class GenericFunction;

	template<class T>
	struct GenericArgumentProperty{
		size_t read_timestamp, write_timestamp;
		bool read_sensitivity, write_sensitivity;
		std::pair<bool /* locked */, const GenericFunction<T>* /* function that has the key */> status;
		size_t lock_timestamp, unlock_timestamp;
	};
	
	/* 
	 * Generic Argument
	 */
	template<class T>
	class GenericArgument{
		using sens_t = char;

		private:
		std::vector<std::pair<T /* arg */, GenericArgumentProperty<T> /* property */>> m_args;

		public:
		/* Init API */
		inline void add(const T& arg, sens_t sensitivity) { 
			if(sensitivity < INSENSITIVE || sensitivity > SENSITIVE_ALL)
				return ;

			m_args.push_back(
					std::make_pair(
						arg, 
						GenericArgumentProperty<T>{ 
							0, 0, 
							sensitivity==SENSITIVE_READ || sensitivity==SENSITIVE_ALL, 
							sensitivity==SENSITIVE_WRITE || sensitivity==SENSITIVE_ALL, 
							std::make_pair(false, nullptr), 
							(size_t)-1, (size_t)-1
						}));
		}

		inline void set(const T& arg, sens_t sensitivity){
			m_args.clear();
			m_args.push_back(
					std::make_pair(
						arg, 
						GenericArgumentProperty<T>{ 
							0, 0, 
							sensitivity==SENSITIVE_READ || sensitivity==SENSITIVE_ALL, 
							sensitivity==SENSITIVE_WRITE || sensitivity==SENSITIVE_ALL, 
							std::make_pair(false, nullptr), 
							(size_t)-1, (size_t)-1
						}));
		}

		inline void remove(size_t index=-1) {
			if(index >= m_args.size())
				m_args.pop_back();
			else
				m_args.erase(m_args.begin()+index);
		}

		inline size_t count() const { return m_args.size(); }

		inline void clear() { m_args.clear(); }

		/* Read/Write API */
		const T* get_readable(size_t index, size_t timestamp){
			if(index >= m_args.size())
					return nullptr;
			if(m_args[index].second.read_sensitivity && 
			   m_args[index].second.read_timestamp >= timestamp)
				return nullptr;
			m_args[index].second.read_timestamp = timestamp;
			return &m_args[index].first;
		}

		const T* get_readable(size_t timestamp){
			return get_readable(0, timestamp);
		}

		T* get_writable(size_t index, size_t timestamp){
			if(index >= m_args.size())
				return nullptr;
			if(m_args[index].second.write_sensitivity && 
			   m_args[index].second.write_timestamp >= timestamp)
				return nullptr;
			m_args[index].second.write_timestamp = timestamp;
			return &m_args[index].first;
		}

		T* get_writable(size_t timestamp){
			return get_writable(0, timestamp);
		}

		/* (Un)Locking API */
		const std::pair<bool, const GenericFunction<T>*>* lock_status(size_t index) const { 
			if(index >= m_args.size())
				return nullptr;
			return &(m_args[index].second.status);
		}

		bool lock(size_t index, const GenericFunction<T>* generic_function, size_t timestamp){
			if(index >= m_args.size() || !generic_function || m_args[index].second.status.first)
				return false;
			m_args[index].second.status = { true, generic_function };
			m_args[index].second.lock_timestamp = timestamp;
			return true;
		}

		bool unlock(size_t index, const GenericFunction<T>* generic_function, size_t timestamp){
			if(index >= m_args.size() || !generic_function || !m_args[index].second.status.first)
				return false;
			m_args[index].second.status = { false, generic_function };
			m_args[index].second.unlock_timestamp = timestamp;
			return true;
		}
	};

	template<class>
	class Engine;

	/*
	 * Generic Function class
	 * Utilizes the generic argument in which changes can be made.
	 * 'run' virt. method has to be defined by the user since this function is called 
	 * through this function by the PPEE.
	 */
	template<class T>
	class GenericFunction{
		static size_t generic_function_count;

		private:
		size_t m_id;
		mutable std::string m_name;
		GenericArgument<T> m_state;

		public:
		explicit GenericFunction(const std::string& name=""):
			m_id(GenericFunction::generic_function_count++),
			m_name(name)
		{
			// ...
		}
		virtual ~GenericFunction(){}

		inline const size_t& id() const { return m_id; }
		inline std::string& name() const { return m_name; }
		inline virtual GenericArgument<T>& state() { return m_state; }

		// This function has to be implemented explicitly by the user!
		// Returns the number of modified generic argument units (optional)
		virtual size_t run(GenericArgument<T>& generic_argument, const Engine<T>* const engine) = 0;
	};

	template<class T>
	size_t GenericFunction<T>::generic_function_count = 0;
	
	enum class ExecutionStyle {		// a -> b -> c
		DEFAULT = 0,				// a -> b -> c
		CYCLIC,						// a -> b -> c -> a -> b -> c -> ...
		REVERSED,					// a -> b -> c -> b -> a -> ...
		RANDOM,						// b -> a -> c -> c -> b -> c -> a -> ...
	};

	/*
	 * API for Execution Path
	 * Execution Path defines the timestamps in which generic functions are going
	 * to be executed.
	 */
	template<class T>
	class ExecutionGraph{
		private:
		std::map<size_t /* timestamp */, std::vector<GenericFunction<T>*> /* functions */> m_function_timestamps;
		std::vector<std::vector<GenericFunction<T>*>> m_generic_functions;
		// ExecutionGraph *prev, *next;
		size_t m_index = 0, m_min_timestamp = 0, m_max_timestamp = 0;
		ExecutionStyle m_execution_style = ExecutionStyle::DEFAULT;
		bool m_randomized_execution = false;

		public:
		void add(GenericFunction<T>* function, size_t timestamp=-1){
			if(!function) return;

			if(timestamp == -1){
				m_function_timestamps[++m_max_timestamp].push_back(function);
			} else{
				m_function_timestamps[timestamp].push_back(function);
				if(timestamp > m_max_timestamp)	m_max_timestamp = timestamp;
			}	
		}

		void remove(size_t timestamp, const GenericFunction<T>* function=nullptr){
			auto& generic_function_vec_it = m_generic_functions.find(timestamp);
			if(!function){
				m_generic_functions.erase(generic_function_vec_it);
			} else{
				auto generic_function_it = generic_function_vec_it->find(function);
				generic_function_vec_it->erase(generic_function_it);
			}
		}

		void clear(){
			m_function_timestamps.clear();
		}

		void compile(bool randomize=false){
			m_generic_functions.clear();
			for(auto it=m_function_timestamps.begin(); it!=m_function_timestamps.end(); ++it){
				if(randomize)	std::random_shuffle(it->second.begin(), it->second.end());
				m_generic_functions.push_back(it->second);
			}
			
		}
		
		void compile(const std::vector<std::vector<GenericFunction<T>*>> generic_functions, bool randomize=false){
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

		// recommended: this function respects the execution style
		std::vector<GenericFunction<T>*>* get_curr(int direction=1){
			if(m_index == -1) return nullptr;

			if(m_randomized_execution)
				std::random_shuffle(m_generic_functions[m_index].begin(), m_generic_functions[m_index].end());	

			auto generic_function_vec = &m_generic_functions[m_index];
			if(m_execution_style == ExecutionStyle::CYCLIC || 
			  (m_execution_style == ExecutionStyle::DEFAULT && m_index+direction < m_generic_functions.size())){
				m_index = (m_index + direction) % m_generic_functions.size();
			} else if(m_execution_style == ExecutionStyle::REVERSED){
				static char dir_sign = 1;

				size_t gap = (dir_sign == 1 ? m_generic_functions.size() - 1 - m_index : m_index);
				size_t count = (abs(direction) - gap) / (m_generic_functions.size() - 1);
				size_t dir = (abs(direction) - gap) % (m_generic_functions.size() - 1);
				if(gap < abs(direction) && !(count % 2))	dir_sign *= -1;

				m_index = (gap >= abs(direction) ? 
					m_index + dir_sign * abs(direction) : 
					(dir_sign == 1 ?
					 	dir :
						m_generic_functions.size() - 1 - dir
					)
				);
			} else if(m_execution_style == ExecutionStyle::RANDOM){
				m_index = rand() % m_generic_functions.size();
			} else m_index = -1;
			return generic_function_vec;
		}

		// warning: this function doesn't respect the execution sytle
		std::vector<GenericFunction<T>*>* get(size_t index){
			if(index >= m_generic_functions.size()){
				// Index out of range handling
				return nullptr;
			}
			m_index = index;
			if(m_randomized_execution)
				std::random_shuffle(m_generic_functions[index].begin(), m_generic_functions[index].end());
			return &m_generic_functions[index];
		}

		// warning: this function doesn't respect the execution sytle
		std::vector<GenericFunction<T>*>* get_prev(){
			size_t tmp = m_index;
			m_index = (m_index-1) % m_generic_functions.size();
			if(m_execution_style == ExecutionStyle::DEFAULT && !tmp){
				return nullptr;
			}
			return get((tmp-1) % m_generic_functions.size());
		}

		// warning: this function doesn't respect the execution sytle
		std::vector<GenericFunction<T>*>* get_next(){
			size_t tmp = m_index;
			m_index = (m_index+1) % m_generic_functions.size();
			if(m_execution_style == ExecutionStyle::DEFAULT && tmp+1 == m_generic_functions.size()){
				return nullptr;
			}
			return get((tmp+1) % m_generic_functions.size());
		}
	};

	void log(const char* restrict_format, ...);

	/*
	 * Engine class
	 */
	template<class T>
	class Engine{
		friend void log(const char* restrict_format, ...);
		
		private:
		ExecutionGraph<T> m_execution_graph;
		GenericArgument<T> m_generic_argument;
		size_t m_timestamp;

		// file path into which logging info will be put
		std::string m_log_file_path;
		mutable FILE* m_log_file;

		struct Diagnostics{
			float activation_per_function;
			float read_per_argument, write_per_argument;
			float activation_density;
		}m_diagnostics;

		public:
		struct Options{
			// degree of verbosity/logging/debugging
			char verbosity;
			// number of consecutive timestamps after which verbosity will occur
			size_t timestamp_steps;
		}opts;

		public:
		Engine():
			m_timestamp(0),
			m_log_file_path("default.log"),
			m_log_file(nullptr),
			opts({ VERBOSITY_DEFAULT, 1 })
		{
			log_file_path(m_log_file_path);
		}
		Engine(const ExecutionGraph<T>& execution_graph, const GenericArgument<T>& generic_argument, const std::string& log_file_path="default.log"):
			m_execution_graph(execution_graph),
			m_generic_argument(generic_argument),
			m_timestamp(0),
			m_log_file_path(log_file_path),
			m_log_file(nullptr),
			opts({ VERBOSITY_DEFAULT, 1 })
		{
			// ...
			this->log_file_path(log_file_path);
		}
		~Engine(){
			if(!m_log_file && m_log_file != stdout && m_log_file != stderr)
				fclose(m_log_file);
		}

		inline ExecutionGraph<T>& execution_graph() { return m_execution_graph; }
		inline GenericArgument<T>& generic_argument() { return m_generic_argument; }
		inline size_t timestamp() const { return m_timestamp; }
		inline const Diagnostics& diagnostics() const { return m_diagnostics; }

		bool log_file_path(const std::string& file_path){
			m_log_file_path = file_path;
			if(m_log_file && m_log_file != stdout && m_log_file != stderr){
				fclose(m_log_file);
				m_log_file = nullptr;
			}

			m_log_file = fopen(m_log_file_path.c_str(), "w");
			if(!m_log_file){
				printf("ERROR: Log file %s couldn't be opened!\n", m_log_file_path.c_str());
				return false;
			}
			return true;
		}
		inline void log_file_stdout() { m_log_file = stdout; }
		inline void log_file_stderr() { m_log_file = stderr; }

		void log(const char* restrict_format, ...) const {
			if(opts.verbosity == VERBOSITY_ALL){
				va_list args;
				va_start(args, restrict_format);
				vfprintf(m_log_file, restrict_format, args);
				va_end(args);
			}
		}
		void log(bool (*filter)(const void* const /* args */), const void* const args, const char* restrict_format, ...) const {
			if(filter && filter(args) && opts.verbosity == VERBOSITY_ALL){
				va_list args;
				va_start(args, restrict_format);
				vfprintf(m_log_file, restrict_format, args);
				va_end(args);
			}
		}

		// runs for n iterations
		int iterate(size_t n=1){
			if(is_invalid()) return -1;
			return iterate_unsafely(n);
		}
		// iterates till an error occurs
		int iterate_all(){
			// TODO
			int ret_val = 0;
			while(!is_invalid() && is_iterable() && ret_val != -1){
				ret_val = iterate_unsafely();
				if(ret_val == -1){
					;
				} else if(ret_val == -2){
					;
				} else if(ret_val < -2){
					;
				}
			}
			return 0;
		}
		// runs till the end of execution; may not halt for a long time
		int run(){
			// TODO
			return 0;
		}

		private:
		inline bool is_invalid() const { return false; }
		inline bool is_iterable() const { return false; }

		int iterate_unsafely(size_t n=1){
			while(n--){
				++m_timestamp;
				// fprintf(stdout, " > Timestamp: %zu\n", m_timestamp);
				if(!(m_timestamp % opts.timestamp_steps) || m_timestamp == 1){
					fprintf(m_log_file, "= = = = = = = = =\n");
					switch(opts.verbosity){
						case VERBOSITY_ALL:
							// fprintf(m_log_file, " >>> Timestamp: %zu\n", m_timestamp);
						case VERBOSITY_NORMAL:
							// fprintf(m_log_file, " >> Timestamp: %zu\n", m_timestamp);
						case VERBOSITY_DEFAULT:
							fprintf(m_log_file, " > Timestamp: %zu\n", m_timestamp);
						default: break;
					}
					fprintf(m_log_file, "= = = = = = = = =\n");
				}

				auto generic_functions = m_execution_graph.get_curr();	
				if(!generic_functions)	return n;

				for(size_t i=0; i<generic_functions->size(); ++i){
					(*generic_functions)[i]->run(m_generic_argument, this);
				}
			}
			return n;
		}
	};
}
 
