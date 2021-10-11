
#pragma once

#include "core.h"

namespace alc{ namespace utils{
	using namespace core;

	std::shared_ptr<Core> find_core_by_id(size_t id, const std::vector<std::shared_ptr<Core>>& cores);
	std::shared_ptr<Core> find_core_by_hash(size_t hash, 
			const std::vector<std::shared_ptr<Core>>& cores);
	std::shared_ptr<Core> find_core_by_name(const std::string& name, 
			const std::vector<std::shared_ptr<Core>>& cores);
} }

