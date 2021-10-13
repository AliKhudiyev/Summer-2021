
#pragma once

#include <string>

#define LOAD_OK false
#define SAVE_OK false

namespace alc{ namespace parameters{
	/*
	 * Options are used to give directions to the ALC algorithm during the runtime.
	 */
	struct Options{
		static const char LOG_NONE = 0;
		static const char LOG_LAST = 1;
		static const char LOG_ALL = 1 << 1;
		static const char LOG_CUSTOM = 1 << 2;

		std::string system_path = "out.sys", options_path = "out.opt";
		std::string data_path, stats_path = "out.sta";
		size_t log_after_iterations = -1;
		float log_after_time = -1.f;
		float log_if_compression_ratio = -1.f;
		size_t delay_ms = 0;

		bool log_core_memory = false;
		bool log_system_memory = false;
		char log_level = LOG_LAST;
	};

	struct Policy{
		static const char OVERWHELMED_FOR_MEMORY = 1;
		static const char OVERWHELMED_FOR_PERFORMANCE = 1 << 1;
		static const char STATIC_COMPRESSION = 1 << 2;
		static const char DYNAMIC_COMPRESSION = 1 << 3;
		
		char policy = STATIC_COMPRESSION | DYNAMIC_COMPRESSION | OVERWHELMED_FOR_MEMORY;
		// if memory usage is over `memory` GBs then system gets overwhelmed
		float overwhelming_memory = 0.5f;
		// if compression ratio is less than the `performance` then system gets overwhelmed
		float overwhelming_performance = 0.f;

		static const char ADAPTIVE_TO_LEARNING_SENSITIVITY = 1;
		static const char ADAPTIVE_TO_AGGRESSIVENESS = 1 << 1;
		static const char ADAPTIVE_TO_CURIOSITY = 1 << 2;
		
		// controls fast-learning tendency by creating a separate sub-structure for each given input
		float learning_sensitivity = 1.f;
		// controls tendency for the use of specific sub-structure(s) more than others
		float tolerance = 0.5f;
		// controls request frequency
		float curiosity = 0.0f;
		// controls lossiness of the compression
		float lossiness = 0.f;
		// controls the size of core STM
		size_t core_memory_size = 16;
		// controls the size of global system memory
		size_t system_memory_size = 1e6;

		// controls adaptiveness
		char adaptiveness = 0;

		// controls the number of sets to recompress
		size_t recompression_depth = 0;
		// controls the number of times recompress all the sets
		size_t recompression_count = 0;
		// controls the frequence of recompression by checking (`freq` * `core_memory_size`)
		float recompression_cooldown = -1.f;
		// controls the randomness of set selection of recompression
		bool random_recompression = false;
		// radius of dynamic compression
		size_t compression_radius = -1;

		float minimum_effectively_compressed = 0.f;
		size_t patience = 10;
		
		size_t epochs = 1;
		size_t max_iteration_count = -1;
		// controls termination criterion (in seconds)
		float max_run_time = -1.f;
		
		/* Interconnect Policies */
		static const char TIME_BASED_UPDATE = 1;
		static const char SUPPORT_BASED_UPDATE = 1 << 1;

		char interconnect_policy;
		// controls decisiveness of system to get rid of useless interconnects asap
		float aggressiveness = 1.f;
		// after each `cooldown` time passes, weak interconnects get removed
		float cooldown = -1.f;
		// if support of an interconnect is more than `threshold`, other relevant interconnects get removed after `cooldown` seconds
		float minimum_support_threshold = 1.f;
		// support penalty when an interconnect cannot be used to predict the true output
		float inconsistency_penalty = 0.f;
	};

	struct Stats{
		float statically_compressed = 0.f;
		float dynamically_compressed = 0.f;
		float effectively_compressed = 0.f;
		float compressed = 0.f;

		size_t total_predictions = 0;
		size_t mistakes = 0;
	};
} }

