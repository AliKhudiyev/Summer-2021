#include "core.h"

namespace alc{ namespace core{
	InterconnectInfo Interconnect::get_info(const std::shared_ptr<Core>& core1, 
			const std::shared_ptr<Core>& core2){
		return core1->interconnect_info(core2);
	}

	size_t Core::core_count = 0;
	std::hash<std::string> Core::hash_string;

	size_t Core::id() const{
		return m_id;
	}

	size_t Core::hash() const{
		return m_hash;
	}

	size_t Core::depth() const{
		return m_depth;
	}

	void Core::set_name(const std::string& name){
		m_name = name;
		if(m_type == CoreType::DEFAULT){
			m_hash = Core::hash_string(m_name);
			update();
		}
	}

	void Core::set_memory(const stm_t& memory){
		m_memory = memory;
	}

	void Core::reset_memory(bool bit){
		for(size_t i=0; i<m_memory.size(); ++i)
			m_memory[i] = bit;
	}

	float Core::get_memory_usage() const{
		float mem = 0.f;
		mem += sizeof(m_id) + sizeof(m_hash) + sizeof(m_depth) + sizeof(m_type) + sizeof(m_name) +
			sizeof(m_output) + sizeof(m_inputs) + m_inputs.size()/8 + sizeof(m_backward) + 
			sizeof(m_forward) + (sizeof(interconnect_t) + sizeof(std::weak_ptr<Core>) + 
					sizeof(InterconnectInfo)) * (m_backward.size() + m_forward.size()) + 
			sizeof(m_memory) + m_memory.size()/8;
		return mem;
	}

	const std::vector<Core::interconnect_t>& Core::backward_connections() const{
		return m_backward;
	}

	const std::vector<Core::interconnect_t>& Core::forward_connections() const{
		return m_forward;
	}

	const InterconnectInfo& Core::interconnect_info(const std::shared_ptr<Core>& core) const{
		auto it = std::find_if(m_forward.begin(), m_forward.end(),
				[core](const interconnect_t& interconnect){
					return interconnect.first.lock() == core;
				});
		if(it == m_forward.end()){
			fprintf(stderr, "interconnect_info failed\n");
			exit(1);
		}
		return it->second;
	}

	bool Core::connected(const std::shared_ptr<Core>& core) const{
		auto it = std::find_if(m_forward.begin(), m_forward.end(),
				[core](const interconnect_t& interconnect){
					return interconnect.first.lock() == core;
				});
		return it != m_forward.end();
	}

	bool Core::connect(const std::shared_ptr<Core>& core){
		if(!connected(core)){
			m_forward.push_back(std::make_pair(core, InterconnectInfo()));
			return core->accept_connection(shared_from_this());
		}
		return false;
	}

	bool Core::disconnect(const std::shared_ptr<Core>& core){
		auto it = std::find_if(m_forward.begin(), m_forward.end(), 
				[core](const interconnect_t& interconnect){
					return interconnect.first.lock() == core;
				});
		if(it != m_forward.end()){
			m_forward.erase(it);
			return core->refuse_connection(shared_from_this());
		}
		return false;
	}

	void Core::set(bool bit){
		set(bit, false);
	}

	bool Core::get() const{
		return m_output;
	}

	bool Core::pull() const{
		BitStream inputs(m_backward.size());
		for(size_t i=0; i<m_backward.size(); ++i){
			if(!m_backward[i].second.speculative)
				inputs[i] = m_backward[i].first.lock()->pull();
		}
		return get(inputs);
	}

	bool Core::propagate_get(){
		for(size_t i=0; i<m_backward.size(); ++i){
			if(!m_backward[i].second.speculative)
				m_inputs[i] = m_backward[i].first.lock()->propagate_get();
		}
		set(get(m_inputs), true);
		return m_output;
	}

	bool Core::propagate(bool stm){
		set(get(m_inputs), stm);
		for(auto& interconnect: m_forward){
			if(!interconnect.second.speculative)
				interconnect.first.lock()->receive(m_output, shared_from_this());
		}
		return m_output;
	}

	bool Core::replaceable_with(const std::shared_ptr<Core>& core) const{
		// TODO: optimize!
		if(m_backward.size() != core->m_backward.size())
			return false;

		for(const auto& core1: m_backward){
			for(const auto& core2: core->m_backward){
				if(core1.first.lock() != core2.first.lock())
					return false;
			}
		}

		return true;
	}

	bool Core::replace_with(std::shared_ptr<Core> core){
		// const char* types[] = { "io", "selector_0", "selector_1", "not" };
		// printf("replacing %s(%zu) with %s(%zu)...\n", 
		// 		types[m_type], m_id, types[core->m_type], core->m_id);
		for(auto& core_weak: m_forward){
			auto core_ = core_weak.first.lock();
			disconnect(core_);
			core->connect(core_);
		}
		// printf("cleaning up...\n");
		remove_if_not_connected();
		return true;
	}

	void Core::remove_if_not_connected(){
		// static int iter = 0;
		// ++iter;
		//if(iter == 8)
		//	exit(1);
		// const char* types[] = { "io", "selector<0>", "selector<1>", "not" };
		// printf("%s(%zu).remove_if_not_connected()\n", types[m_type], m_id);
		if(m_forward.empty() && m_type != CoreType::DEFAULT){
			// printf("| pseudo-erasing %s(%zu) from cores...\n", types[m_type], m_id);
			for(size_t i=0; i<m_backward.size(); ){
				// printf("\titerating from %s(%zu)\n", types[m_type], m_id);
				// if(i >= m_backward.size()){
				// 	printf("size wtf!\n");
				// 	exit(1);
				// }
				auto core = m_backward[i].first.lock();
				// if(!core){
				// 	printf("wtf?!\n");
				// 	exit(1);
				// } else{
				// 	printf(";; %s(%zu)->disconnect(%s(%zu))\n",
				// 			types[core->m_type], core->m_id,
				// 			types[m_type], m_id);
				// }
				core->disconnect(shared_from_this());
				core->remove_if_not_connected();
				// --i;
			}
			// printf("\tdone from %s(%zu)\n", types[m_type], m_id);
		}
		// else
		// 	printf("| no need to erase... done from %s(%zu)\n", types[m_type], m_id);
	}

	void Core::set(bool bit, bool stm){
		for(size_t i=0; i<m_inputs.size(); ++i)
			m_inputs[i] = bit;
		m_output = bit;
		if(stm){
			m_memory.rotate(1, m_memory.begin(), m_memory.end());
			m_memory[0] = m_output;
		}
	}

	bool Core::accept_connection(const std::shared_ptr<Core>& core){
		if((m_type == CoreType::DEFAULT || m_type == CoreType::NEGATOR) && m_backward.size() == 1)
			return false;
		m_inputs.push(false, m_inputs.end());
		m_backward.push_back(std::make_pair(core, InterconnectInfo()));
		update();
		return true;
	}

	bool Core::refuse_connection(const std::shared_ptr<Core>& core){
		auto it = std::find_if(m_backward.begin(), m_backward.end(),
					[core](const interconnect_t& interconnect){
						return interconnect.first.lock() == core;
					});
		if(it == m_backward.end())
			return false;
		m_inputs.pop(m_inputs.begin()+(it-m_backward.begin()), 1);
		m_backward.erase(it);
		update();
		return true;
	}

	void Core::receive(bool bit, const std::shared_ptr<Core>& core){
		size_t index = std::find_if(m_backward.begin(), m_backward.end(), 
				[core](const interconnect_t& interconnect){
					return interconnect.first.lock() == core;
				}) - m_backward.begin();
		m_inputs[index] = bit;

		// const char* types[4] = { "core", "selector<0>", "selector<1>", "not" };
		// printf("%s(%zu) received %d from %s(%zu): %s, [%zu]\n", 
		// 		types[m_type], m_id, (int)bit, types[core->type()], core->id(), 
		// 		m_inputs.to_string().c_str(), index);
	}

	void Core::update(){
		m_depth = 0;
		std::string tmp;
		for(const auto& core: m_backward){
			if(m_depth <= core.first.lock()->depth())
				m_depth = core.first.lock()->depth() + 1;
			tmp += std::to_string(core.first.lock()->hash()) + ".";
		}
		if(m_type != CoreType::DEFAULT)
			m_hash = Core::hash_string(tmp);
		for(auto& core: m_forward){
			core.first.lock()->update();
		}
	}

	bool Core::get(const BitStream& inputs) const{
		if(inputs.size())
			return inputs[0];
		return 0;
	}
} }
