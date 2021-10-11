#include "utils.h"
#include <algorithm>

using namespace alc::core;

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
} }
