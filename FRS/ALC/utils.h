
#pragma once

#include "core.h"
#include "parameters.h"
#include <cstdio>
#include <cstdlib>

namespace alc{ namespace utils{
	using namespace core;
	using namespace parameters;

	std::shared_ptr<Core> find_core_by_id(size_t id, const std::vector<std::shared_ptr<Core>>& cores);
	std::shared_ptr<Core> find_core_by_hash(size_t hash, 
			const std::vector<std::shared_ptr<Core>>& cores);
	std::shared_ptr<Core> find_core_by_name(const std::string& name, 
			const std::vector<std::shared_ptr<Core>>& cores);
	
	void print_params(const Options& opts, FILE* file=stdout);
	void print_params(const Policy& pol, FILE* file=stdout);
	std::string set_param(const char* key_, const char* value, Options& opts, Policy& pol);
	bool is_valid_param(const char* key, Options& opts, Policy& pol);
	std::string get_info(const char* key, Options& opts, Policy& pol);
	bool save_params(const char* file_path, const Options& opts, const Policy& pol);
	bool load_params(const char* file_path, Options& opts, Policy& pol);
} }

