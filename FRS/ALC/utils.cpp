#include "utils.h"
#include <algorithm>

using namespace alc::core;
using namespace alc::parameters;

namespace alc{ namespace utils{
	std::shared_ptr<Core> find_core_by_id(size_t id, const std::vector<std::shared_ptr<Core>>& cores){
		std::shared_ptr<Core> core(nullptr);
		auto it = std::find_if(cores.begin(), cores.end(),
				[id](const std::shared_ptr<Core>& core){
					return core->id() == id;
				});
		if(it != cores.end())
			core = *it;
		return core;
	}

	std::shared_ptr<Core> find_core_by_hash(size_t hash, 
			const std::vector<std::shared_ptr<Core>>& cores){
		std::shared_ptr<Core> core(nullptr);
		auto it = std::find_if(cores.begin(), cores.end(),
				[hash](const std::shared_ptr<Core>& core){
					return core->hash() == hash;
				});
		if(it != cores.end())
			core = *it;
		return core;
	}

	std::shared_ptr<Core> find_core_by_name(const std::string& name, 
			const std::vector<std::shared_ptr<Core>>& cores){
		std::shared_ptr<Core> core(nullptr);
		auto it = std::find_if(cores.begin(), cores.end(),
				[name](const std::shared_ptr<Core>& core){
					return core->name() == name;
				});
		if(it != cores.end())
			core = *it;
		return core;
	}

	void print_params(const Options& opts, FILE* file){
		fprintf(file, "<<< = = = Options = = = >>>\n");
		fprintf(file, "system_path %s\noptions_path %s\ndata_path %s\nstats_path %s\n",
				opts.system_path.c_str(), opts.options_path.c_str(), 
				opts.data_path.c_str(), opts.stats_path.c_str());
		fprintf(file, "log_after_iterations %d\nlog_after_time %f\nlog_if_compression_ratio %f\n", 
				(int)opts.log_after_iterations, opts.log_after_time, opts.log_if_compression_ratio);
		fprintf(file, "delay_ms %d\n", (int)opts.delay_ms);
		fprintf(file, "log_core_memory %d\nlog_system_memory %d\nlog_level %d\n", 
				(int)opts.log_core_memory, (int)opts.log_system_memory, opts.log_level);
		fprintf(file, "\n");
	}

	void print_params(const Policy& pol, FILE* file){
		fprintf(file, "<<< = = = Policy = = = >>>\n");
		fprintf(file, "policy %d\noverwhelming_memory %f\noverwhelming_performance %f\n",
				pol.policy, pol.overwhelming_memory, pol.overwhelming_performance);
		fprintf(file, "learning_sensitivity %f\ntolerance %f\ncuriosity %f\nlossiness %f\n",
				pol.learning_sensitivity, pol.tolerance, pol.curiosity, pol.lossiness);
		fprintf(file, "core_memory_size %d\nsystem_memory_size %d\nadaptiveness %d\n",
				(int)pol.core_memory_size, (int)pol.system_memory_size, pol.adaptiveness);
		fprintf(file, "recompression_depth %d\nrecompression_count %d\nrecompression_cooldown %f\n",
				(int)pol.recompression_depth, (int)pol.recompression_count, pol.recompression_cooldown);
		fprintf(file, "random_recompression %d\ncompression_radius %d\n",
				(int)pol.random_recompression, (int)pol.compression_radius);
		fprintf(file, "minimum_effectively_compressed %f\npatience %d\n",
				pol.minimum_effectively_compressed, (int)pol.patience);
		fprintf(file, "epochs %d\nmax_iteration_count %d\nmax_run_time %f\n",
				(int)pol.epochs, (int)pol.max_iteration_count, pol.max_run_time);
		fprintf(file, "interconnect_policy %d\naggressiveness %f\ncooldown %f\nminimum_support_threshold %f\n",
				pol.interconnect_policy, pol.aggressiveness, pol.cooldown, pol.minimum_support_threshold);
		fprintf(file, "inconsistency_penalty %f\n", pol.inconsistency_penalty);
		fprintf(file, "\n");
	}

	// returns info for `key`
	std::string set_param(const char* key_, const char* value, Options& opts, Policy& pol, bool unset){
		std::string info;
		std::string key(key_);

		// Options
		if(key == "system_path"){
			info = "string, system output file path";
			if(value)
				opts.system_path = std::string(value);
		}
		else if(key == "options_path"){
			info = "string, parameters(options+policy) output file path";
			if(value)
				opts.options_path = std::string(value);
		}
		else if(key == "data_path"){
			info = "string, dataset output file path";
			if(value)
				opts.data_path = std::string(value);
		}
		else if(key == "stats_path"){
			info = "string, statistics output file path";
			if(value)
				opts.stats_path = std::string(value);
		}
		else if(key == "log_after_iterations"){
			info = "size_t, after this many iterations everything's logged, -1 to discard";
			if(value)
				opts.log_after_iterations = (size_t)atol(value);
		}
		else if(key == "log_after_time"){
			info = "float, after this much milliseconds everything's logged, -1 to discard";
			if(value)
				opts.log_after_time = atof(value);
		}
		else if(key == "log_if_compression_ratio"){
			info = "float, if effective compression ratio is bigger than this threshold then everything's logged, -1 to discard";
			if(value)
				opts.log_if_compression_ratio = atof(value);
		}
		else if(key == "delay_ms"){
			info = "size_t, the program pauses this much milliseconds between iterations, 0 to no pause";
			if(value)
				opts.delay_ms = (size_t)atol(value);
		}
		else if(key == "log_core_memory"){
			info = "bool, true(1) to log core memory and false(0) to not log";
			if(value)
				opts.log_core_memory = atoi(value);
		}
		else if(key == "log_system_memory"){
			info = "bool, true(1) to log system memory and false(0) to not log";
			if(value)
				opts.log_system_memory = atoi(value);
		}
		else if(key == "log_level"){
			info = "char, 0(LOG_NONE), 1(LOG_LAST), 2(LOG_ALL), 4(LOG_CUSTOM), default is 1";
			if(value)
				opts.log_level = (char)atoi(value);
		}

		// Policy
		else if(key == "policy"){
			info = "char, 1(OVERWHELMED_FOR_MEMORY), 2(OVERWHELMED_FOR_PERFORMANCE), 4(STATIC_COMPRESSION), 8(DYNAMIC_COMPRESSION), default is (OVERWHELMED_FOR_MEMORY | STATIC_COMPRESSION | DYNAMIC_COMPRESSION)";
			if(value){
				std::string val(value);
				if(val[0] > '9'){
					if(unset){
						if(val == "OVERWHELMED_FOR_MEMORY")
							pol.policy &= ~Policy::OVERWHELMED_FOR_MEMORY;
						else if(val == "OVERWHELMED_FOR_PERFORMANCE")
							pol.policy &= ~Policy::OVERWHELMED_FOR_MEMORY;
						else if(val == "STATIC_COMPRESSION")
							pol.policy &= ~Policy::STATIC_COMPRESSION;
						else if(val == "DYNAMIC_COMPRESSION")
							pol.policy &= ~Policy::DYNAMIC_COMPRESSION;
						else
							fprintf(stderr, "Value not found!\n");
					}
					else{
						if(val == "OVERWHELMED_FOR_MEMORY")
							pol.policy |= Policy::OVERWHELMED_FOR_MEMORY;
						else if(val == "OVERWHELMED_FOR_PERFORMANCE")
							pol.policy |= Policy::OVERWHELMED_FOR_PERFORMANCE;
						else if(val == "STATIC_COMPRESSION")
							pol.policy |= Policy::STATIC_COMPRESSION;
						else if(val == "DYNAMIC_COMPRESSION")
							pol.policy |= Policy::DYNAMIC_COMPRESSION;
						else
							fprintf(stderr, "Value not found!\n");
					}
				}
				else{
					if(unset)
						pol.policy &= ~(char)atoi(value);
					else
						pol.policy |= (char)atoi(value);
				}
			}
		}
		else if(key == "overwhelming_memory"){
			info = "float, ";
			if(value)
				pol.overwhelming_memory = atof(value);
		}
		else if(key == "overwhelming_performance"){
			info = "float, ";
			if(value)
				pol.overwhelming_performance = atof(value);
		}
		else if(key == "learning_sensitivity"){
			info = "float [0, 1], controls fast-learning by creating separate sub-structure for each given input, default is 1";
			if(value){
				pol.learning_sensitivity = atof(value);
				if(pol.learning_sensitivity > 1)
					pol.learning_sensitivity = 1;
				else if(pol.learning_sensitivity < 0)
					pol.learning_sensitivity = 0;
			}
		}
		else if(key == "tolerance"){
			info = "float [0, 1], ";
			if(value){
				pol.tolerance = atof(value);
				if(pol.tolerance > 1)
					pol.tolerance = 1;
				else if(pol.tolerance < 0)
					pol.tolerance = 0;
			}	
		}
		else if(key == "curiosity"){
			info = "float [0, 1], ";
			if(value){
				pol.curiosity = atof(value);
				if(pol.curiosity > 1)
					pol.curiosity = 1;
				else if(pol.curiosity < 0)
					pol.curiosity = 0;
			}
		}
		else if(key == "lossiness"){
			info = "float [0, 1], lossiness of compression, default is 0 or lossless";
			if(value){
				pol.lossiness = atoi(value);
				if(pol.lossiness > 1)
					pol.lossiness = 1;
				else if(pol.lossiness < 0)
					pol.lossiness = 0;
			}
		}
		else if(key == "core_memory_size"){
			info = "size_t, bit count of internal core memory, default is 16";
			if(value)
				pol.core_memory_size = (size_t)atol(value);
		}
		else if(key == "system_memory_size"){
			info = "size_t, bit count of system memory, default is 1e6";
			if(value)
				pol.system_memory_size = (size_t)atol(value);
		}
		else if(key == "adaptiveness"){
			info = "char, ";
			if(value)
				pol.adaptiveness = (char)atoi(value);
		}
		else if(key == "recompression_depth"){
			info = "size_t, ";
			if(value)
				pol.recompression_depth = (size_t)atol(value);
		}
		else if(key == "recompression_count"){
			info = "size_t, ";
			if(value)
				pol.recompression_count = (size_t)atol(value);
		}
		else if(key == "recompression_cooldown"){
			info = "float, ";
			if(value)
				pol.recompression_cooldown = atof(value);
		}
		else if(key == "random_recompression"){
			info = "bool, ";
			if(value)
				pol.random_recompression = atoi(value);
		}
		else if(key == "compression_radius"){
			info = "size_t, radius of dynamic compression, -1 for full system diagnostics";
			if(value)
				pol.compression_radius = (size_t)atol(value);
		}
		else if(key == "minimum_effectively_compressed"){
			info = "float [0, 1], determines convergent behaviour of the program, default is 0";
			if(value)
				pol.minimum_effectively_compressed = atof(value);
		}
		else if(key == "patience"){
			info = "size_t, after this many convergent iterations program terminates, -1 to discard";
			if(value)
				pol.patience = (size_t)atol(value);
		}
		else if(key == "epochs"){
			info = "size_t, number of full-cycle iterations on the given dataset, default is 1";
			if(value)
				pol.epochs = (size_t)atol(value);
		}
		else if(key == "max_iteration_count"){
			info = "size_t, after this many iterations program terminates, -1 to discard";
			if(value)
				pol.max_iteration_count = (size_t)atol(value);
		}
		else if(key == "max_run_time"){
			info = "float, after this much seconds program terminates, -1 to discard";
			if(value)
				pol.max_run_time = atof(value);
		}
		else if(key == "interconnect_policy"){
			info = "char, 1(TIME_BASED_UPDATE), 2(SUPPORT_BASED_UPDATE)";
			if(value)
				pol.interconnect_policy = (char)atoi(value);
		}
		else if(key == "aggressiveness"){
			info = "float [0, 1], controls decisiveness of system to get rid of useless interconnects asap, default is 1";
			if(value)
				pol.aggressiveness = atof(value);
		}
		else if(key == "cooldown"){
			info = "float, after this much milliseconds weak interconnects get removed, -1 to discard";
			if(value)
				pol.cooldown = atof(value);
		}
		else if(key == "minimum_support_threshold"){
			info = "float, ";
			if(value)
				pol.minimum_support_threshold = atof(value);
		}
		else if(key == "inconsistency_penalty"){
			info = "float, effectively wrong interconnects get this support penalty, default is 0";
			if(value)
				pol.inconsistency_penalty = atof(value);
		}

		return info;
	}

	bool is_valid_param(const char* key, Options& opts, Policy& pol){
		return set_param(key, 0, opts, pol).size();
	}

	std::string get_info(const char* key, Options& opts, Policy& pol){
		return set_param(key, 0, opts, pol);
	}

	bool save_params(const char* file_path, const Options& opts, const Policy& pol){
		FILE* file = fopen(file_path, "w");
		if(!file){
			fprintf(stderr, "Could not open file [%s]!\n", file_path);
			return !SAVE_OK;
		}
		print_params(opts, file);
		print_params(pol, file);
		fclose(file);
		return SAVE_OK;
	}

	bool load_params(const char* file_path, Options& opts, Policy& pol){
		FILE* file = fopen(file_path, "r");
		if(!file){
			fprintf(stderr, "Could not open file [%s]!\n", file_path);
			return !LOAD_OK;
		}

		size_t len = 100;
		char* key = (char*)malloc(100);
		char* value = (char*)malloc(100);
		
		// Options
		if(getline(&key, &len, file) == -1)
			return !LOAD_OK;

		for(int i=0; i<11; ++i){
			getdelim(&key, &len, ' ', file);
			getline(&value, &len, file);
			key[strlen(key)-1] = 0;
			value[strlen(value)-1] = 0;
			set_param(key, value, opts, pol);
		}

		// Policy
		if(getline(&key, &len, file) == -1 || getline(&key, &len, file) == -1)
			return !LOAD_OK;

		for(int i=0; i<25; ++i){
			fscanf(file, "%s%s", key, value);
			set_param(key, value, opts, pol);
		}

		free(key);
		free(value);
		fclose(file);
		return LOAD_OK;
	}
} }
