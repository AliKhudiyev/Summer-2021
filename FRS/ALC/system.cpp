#include "system.h"
#include "io.h"
#include "utils.h"
#include <cstdio>
#include <iterator>
#include <deque>
#include <chrono>
#include <thread>

#define RAND_PROB (urd(mte))

using namespace alc::utils;

namespace alc{
	static std::random_device rd;
	static std::mt19937 mte(rd());
	static std::uniform_real_distribution<float> urd(0.f, 1.f);

	void System::reset(const Policy& policy, const Options& options){
		m_policy = policy;
		m_options = options;

		m_inputs.clear();
		m_outputs.clear();
		m_cores.clear();
	}

	void System::set_policy(const Policy& policy){
		m_policy = policy;
	}

	void System::set_options(const Options& options){
		m_options = options;
	}

	const Policy& System::get_policy() const{
		return m_policy;
	}

	const Options& System::get_options() const{
		return m_options;
	}

	const Stats& System::get_stats() const{
		return m_stats;
	}

	void System::fit(const raw_input_t& input, const raw_output_t& output){
		++m_iteration_count;
		if(predict(input) != output || RAND_PROB < m_policy.learning_sensitivity){
			compress(create(input, output));
		}
	}

	void System::fit(IO& io){
		size_t epoch = 0;
		raw_output_t out_pred;

		while(epoch++ < m_policy.epochs){
			for(size_t i=0; i<io.size(); ++i){
				out_pred = fit_predict(io.input_at(i), io.output_at(i));
				if(!out_pred.size()) break;
			}
		}

		if(m_options.log_level == Options::LOG_LAST){
			save("test.sys", "test.opt", "w");
			m_stats.save(m_options.stats_path.c_str(), "w");
		}
		printf("after: %s\n", to_strings().back().c_str());
	}

	raw_output_t System::fit_predict(const raw_input_t& input, const raw_output_t& output){
		using namespace std::chrono;

		raw_output_t out_pred;
		static const size_t patience = m_policy.patience;
		static float passed_time = 0.f;

		// Termination criterions check
		if(m_iteration_count > m_policy.max_iteration_count ||
				(m_policy.max_run_time != -1.f && m_policy.max_run_time <= passed_time) || 
				!m_policy.patience || get_memory_usage() >= m_policy.system_memory_size){
			printf("Criterion met! Terminating...\n");
			return out_pred;
		}

		// Predicting
		out_pred = predict(input);

		// Fitting
		auto t1 = high_resolution_clock::now();
		fit(input, output);
		auto t2 = high_resolution_clock::now();
		duration<float> time_span = duration_cast<duration<float>>(t2 - t1);
		passed_time += time_span.count();

		// Patience update
		if(m_stats.effectively_compressed < m_policy.minimum_effectively_compressed)
			--m_policy.patience;
		else
			m_policy.patience = patience;

		// Logging
		if(m_options.log_level == Options::LOG_ALL)
			save(m_options.system_path.c_str(), m_options.options_path.c_str(), "a");
		else if(m_options.log_level == Options::LOG_CUSTOM && 
				(!(m_iteration_count % m_options.log_after_iterations) ||
				m_stats.effectively_compressed >= m_options.log_if_compression_ratio ||
				m_options.log_after_time)){
			save(m_options.system_path.c_str(), m_options.options_path.c_str(), "a");
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(m_options.delay_ms));
		return out_pred;
	}

	void System::predict(const raw_input_t& input, raw_output_t& output){
		for(size_t i=0; i<m_inputs.size(); ++i)
			m_inputs[i]->set(input[i]);

		stabilize(false);

		for(size_t i=0; i<m_outputs.size(); ++i)
			output[i] = m_outputs[i]->get();
	}

	raw_output_t System::predict(const raw_input_t& input){
		raw_output_t output(m_outputs.size());
		predict(input, output);
		return output;
	}

	void System::predict(IO& io){
		for(size_t i=0; i<io.size(); ++i){
			raw_input_t input = io.input_at(i);
			io.output_at(i, predict(input));
		}
	}

	void System::save(const char* sys_path, const char* opts_path, const char* mode){
		char mode_[] { "w" };
		if(mode[0] == 'a')
			mode_[0] = 'a';

		FILE* file = fopen(sys_path, mode_);
		if(file){
			fprintf(file, "id, type, subtype, id1, id2, depth, support\n");
			std::vector<std::shared_ptr<Core>> cores = all_cores();
			const char* core_types[] = { "io", "selector_0", "selector_1", "not" };
			const char* interconnect_types[] = { "regular", "speculative" };

			for(const auto& core: cores)
				fprintf(file, "%zu, %s, %s, %zu, %zu, %zu, %.1f\n",
						core->id(), "core", core_types[core->type()], 0ul, 0ul, core->depth(), 0.f);

			update_interconnects();
			for(const auto& interconnect: m_interconnects)
				fprintf(file, "%zu, %s, %s, %zu, %zu, %zu, %.1f\n",
						0ul, "interconnect", interconnect_types[interconnect.info.speculative],
						interconnect.cores.first.lock()->id(), interconnect.cores.second.lock()->id(),
						0ul, interconnect.info.support);

			fclose(file);
		}

		// Options
		file = fopen(opts_path, mode_);
		if(file){
			print_params(m_options, file);
			print_params(m_policy, file);
			fclose(file);
		}
	}

	bool System::load(const char* sys_path, const char* opts_path){
		FILE* file = fopen(sys_path, "r");
		if(!file)
			return !LOAD_OK;

		size_t id, id1, id2;
		char buf[5][25];
		std::map<size_t /* original id */, size_t /* assigned id */> mp;

		fscanf(file, "%s, %s, %s, %s, %s", buf[0], buf[1], buf[2], buf[3], buf[4]);
		while(!feof(file)){
			fscanf(file, "%zu, %s, %s, %zu, %zu", &id, buf[0], buf[1], &id1, &id2);
			std::string type(buf[0]);
			std::string subtype = std::string(buf[1]);

			if(type == "core"){
				std::shared_ptr<Core> core; 

				if(subtype == "selector_0"){
					core = std::static_pointer_cast<Core>(std::make_shared<Selector<0>>());
					m_inputs.push_back(core);
				} else if(subtype == "selector_1"){
					core = std::static_pointer_cast<Core>(std::make_shared<Selector<1>>());
					m_inputs.push_back(core);
				} else if(subtype == "not"){
					core = std::static_pointer_cast<Core>(std::make_shared<NOT>());
					m_inputs.push_back(core);
				} else{
					core = std::make_shared<Core>();
					if(subtype == "input")
						m_inputs.push_back(core);
					else // subtype == "output"
						m_outputs.push_back(core);
				}
			} else{ // type == "interconnect"
				Interconnect interconnect;
				auto core1 = find_core_by_id(id1, CoreSubtype::ALL);
				auto core2 = find_core_by_id(id2, CoreSubtype::ALL);

				core1->connect(core2);
				if(subtype == "speculative")
					interconnect.info.speculative = true;
				interconnect.info.support = 1.f;
				interconnect.cores = std::pair<std::weak_ptr<Core>, std::weak_ptr<Core>>(core1, core2);
				m_interconnects.push_back(interconnect);
			}
		}

		fclose(file);

		// Options
		return load_params(opts_path, m_options, m_policy);
	}

	std::vector<std::string> System::to_strings() const{
		std::vector<std::string> exprs(m_outputs.size());
		for(size_t i=0; i<m_outputs.size(); ++i){
			const auto& cores = m_outputs[i]->backward_connections();
			auto& expr = exprs[i];
			if(cores.empty())
				expr = "[No interconnects!]";
			else{
				// TODO: optimize
				to_string(expr, m_outputs[i]->backward_connections().front().first);
				// printf("result: %s\n", expr.c_str());
				while(1){
					bool is_updated = false;
					size_t indices[2] = { 0 };
					for(size_t i=0; i<expr.size()-1; ++i){
						if(expr[i] == '(' && 
								(expr[i+1] == 'i' || expr[i+1] == 'c' || expr[i+1] == '~')){
							for(size_t j=i+2; j<expr.size(); ++j){
								if(expr[j] == '&' || expr[j] == '|' || 
									expr[j] == ' ' || expr[j] == '(')
									break;
								else if(expr[j] == ')'){
									expr.erase(expr.begin()+j);
									expr.erase(expr.begin()+i);
									is_updated = true;
									i = expr.size();
									// printf("new: %s\n", expr.c_str());
									break;
								}
							}
						}
					}
					if(!is_updated) break;
				}
			}
		}
		return exprs;
	}

	void System::from_strings(const std::vector<std::string>& exprs){
		for(const auto& expr: exprs)
			from_string(expr);
	}

	float System::get_memory_usage() const{
		float mem = 0.f;
		for(const auto& core: m_inputs)
			mem += core->get_memory_usage();
		for(const auto& core: m_outputs)
			mem += core->get_memory_usage();
		for(const auto& core: m_cores)
			mem += core->get_memory_usage();
		mem += sizeof(m_options) + sizeof(m_policy) + sizeof(m_stats) + 
			sizeof(m_input_count) + sizeof(m_output_count) + sizeof(m_iteration_count) +
			sizeof(m_specialized_for) + sizeof(Interconnect) * m_interconnects.size() +
			sizeof(m_memory) + m_memory.get_memory_usage();
		return mem;
	}

	std::shared_ptr<Core> System::find_core_by_id(size_t id, CoreSubtype subtype){
		std::shared_ptr<Core> core(nullptr);
		std::vector<std::shared_ptr<Core>>::iterator it;

		if(subtype == CoreSubtype::INPUT){
			core = utils::find_core_by_id(id, m_inputs);
		}
		else if(subtype == CoreSubtype::OUTPUT){
			core = utils::find_core_by_id(id, m_outputs);
		}
		else if(subtype == CoreSubtype::INPUT_OUTPUT){
			auto core = utils::find_core_by_id(id, m_outputs);
			if(!core)
				core = utils::find_core_by_id(id, m_outputs);
		}
		else if(subtype == CoreSubtype::SELECTOR_0){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[id](const std::shared_ptr<Core>& core){
						return core->id() == id && core->type() == CoreType::SELECTOR_0;
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::SELECTOR_1){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[id](const std::shared_ptr<Core>& core){
						return core->id() == id && core->type() == CoreType::SELECTOR_1;
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::SELECTOR){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[id](const std::shared_ptr<Core>& core){
						return core->id() == id && 
						(core->type() == CoreType::SELECTOR_0 || core->type() == CoreType::SELECTOR_1);
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::NEGATOR){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[id](const std::shared_ptr<Core>& core){
						return core->id() == id && core->type() == CoreType::NEGATOR;
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::FUNCTIONAL){
			core = utils::find_core_by_id(id, m_cores);
		}
		else{ // subtype == CoreSubtype::ALL
			core = utils::find_core_by_id(id, m_cores);
			if(!core)
				core = utils::find_core_by_id(id, m_inputs);
			if(!core)
				core = utils::find_core_by_id(id, m_outputs);

		}
		return core;
	}

	std::shared_ptr<Core> System::find_core_by_hash(size_t hash, CoreSubtype subtype){
		std::shared_ptr<Core> core(nullptr);
		std::vector<std::shared_ptr<Core>>::iterator it;

		if(subtype == CoreSubtype::INPUT){
			core = utils::find_core_by_hash(hash, m_inputs);
		}
		else if(subtype == CoreSubtype::OUTPUT){
			core = utils::find_core_by_hash(hash, m_outputs);
		}
		else if(subtype == CoreSubtype::INPUT_OUTPUT){
			auto core = utils::find_core_by_hash(hash, m_outputs);
			if(!core)
				core = utils::find_core_by_hash(hash, m_outputs);
		}
		else if(subtype == CoreSubtype::SELECTOR_0){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[hash](const std::shared_ptr<Core>& core){
						return core->hash() == hash && core->type() == CoreType::SELECTOR_0;
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::SELECTOR_1){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[hash](const std::shared_ptr<Core>& core){
						return core->hash() == hash && core->type() == CoreType::SELECTOR_1;
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::SELECTOR){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[hash](const std::shared_ptr<Core>& core){
						return core->hash() == hash && 
						(core->type() == CoreType::SELECTOR_0 || core->type() == CoreType::SELECTOR_1);
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::NEGATOR){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[hash](const std::shared_ptr<Core>& core){
						return core->hash() == hash && core->type() == CoreType::NEGATOR;
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::FUNCTIONAL){
			core = utils::find_core_by_hash(hash, m_cores);
		}
		else{ // subtype == CoreSubtype::ALL
			core = utils::find_core_by_hash(hash, m_cores);
			if(!core)
				core = utils::find_core_by_hash(hash, m_inputs);
			if(!core)
				core = utils::find_core_by_hash(hash, m_outputs);

		}
		return core;
	}

	std::shared_ptr<Core> System::find_core_by_name(const std::string& name, CoreSubtype subtype){
		std::shared_ptr<Core> core(nullptr);
		std::vector<std::shared_ptr<Core>>::iterator it;

		if(subtype == CoreSubtype::INPUT){
			core = utils::find_core_by_name(name, m_inputs);
		}
		else if(subtype == CoreSubtype::OUTPUT){
			core = utils::find_core_by_name(name, m_outputs);
		}
		else if(subtype == CoreSubtype::INPUT_OUTPUT){
			auto core = utils::find_core_by_name(name, m_outputs);
			if(!core)
				core = utils::find_core_by_name(name, m_outputs);
		}
		else if(subtype == CoreSubtype::SELECTOR_0){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[name](const std::shared_ptr<Core>& core){
						return core->name() == name && core->type() == CoreType::SELECTOR_0;
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::SELECTOR_1){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[name](const std::shared_ptr<Core>& core){
						return core->name() == name && core->type() == CoreType::SELECTOR_1;
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::SELECTOR){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[name](const std::shared_ptr<Core>& core){
						return core->name() == name && 
						(core->type() == CoreType::SELECTOR_0 || core->type() == CoreType::SELECTOR_1);
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::NEGATOR){
			it = std::find_if(m_cores.begin(), m_cores.end(),
					[name](const std::shared_ptr<Core>& core){
						return core->name() == name && core->type() == CoreType::NEGATOR;
					});
			if(it != m_cores.end())
				core = *it;
		}
		else if(subtype == CoreSubtype::FUNCTIONAL){
			core = utils::find_core_by_name(name, m_cores);
		}
		else{ // subtype == CoreSubtype::ALL
			core = utils::find_core_by_name(name, m_cores);
			if(!core)
				core = utils::find_core_by_name(name, m_inputs);
			if(!core)
				core = utils::find_core_by_name(name, m_outputs);

		}
		return core;
	}

	Interconnect System::find_interconnect(size_t id1, size_t id2, InterconnectSubtype subtype){
		Interconnect interconnect;
		auto it = std::find_if(m_interconnects.begin(), m_interconnects.end(),
				[id1, id2, subtype](const Interconnect& interconnect){
					return (interconnect.cores.first.lock()->id() == id1 && 
							interconnect.cores.second.lock()->id() == id2) || 
							(interconnect.cores.first.lock()->id() == id2 && 
							interconnect.cores.second.lock()->id() == id1 && 
							subtype == InterconnectSubtype::SPECULATIVE);
				});
		if(it != m_interconnects.end())
			interconnect = *it;
		return interconnect;
	}

	std::vector<std::shared_ptr<Core>> System::all_cores() const{
		std::vector<std::shared_ptr<Core>> cores(m_inputs.size() + m_outputs.size() + m_cores.size());
		size_t index = 0;
		for(const auto& core: m_inputs)
			cores[index++] = core;
		for(const auto& core: m_outputs)
			cores[index++] = core;
		for(const auto& core: m_cores)
			cores[index++] = core;
		return cores;
	}

	bool System::make_interconnect(const std::shared_ptr<Core>& core1, 
			const std::shared_ptr<Core>& core2){
		// TODO!!: fix cumulative support values
		auto it = std::find_if(m_interconnects.begin(), m_interconnects.end(),
				[core1, core2](const Interconnect& interconnect){
					return (interconnect.cores.first.lock() == core1 && 
							interconnect.cores.second.lock() == core2) || 
							(interconnect.cores.first.lock() == core2 && 
							 interconnect.cores.second.lock() == core1);
				});
		if(it == m_interconnects.end()){
			Interconnect interconnect;
			interconnect.info.support += 0;
			interconnect.info.speculative = true;
			interconnect.cores = std::pair<std::shared_ptr<Core>, std::shared_ptr<Core>>(core1, core2);
			m_interconnects.push_back(interconnect);
			return true;
		}
		it->info.support += 0;
		return false;
	}

	size_t System::max_depth() const{
		size_t output = 0;
		for(const auto& core: m_outputs){
			if(output < core->depth())
				output = core->depth();
		}
		return output + 1;
	}

	const std::vector<std::shared_ptr<Core>> System::get(size_t depth) const{
		std::vector<std::shared_ptr<Core>> output;
		for(auto& core: m_inputs){
			if(core->depth() == depth)
				output.push_back(core);
		}
		for(auto& core: m_outputs){
			if(core->depth() == depth)
				output.push_back(core);
		}
		for(auto& core: m_cores){
			if(core->depth() == depth)
				output.push_back(core);
		}
		return output;
	}

	void System::propagate(size_t depth) const{
		for(auto& core: m_inputs){
			if(core->depth() == depth)
				core->propagate();
		}
		for(auto& core: m_outputs){
			if(core->depth() == depth)
				core->propagate();
		}
		for(auto& core: m_cores){
			if(core->depth() == depth)
				core->propagate();
		}
	}

	void System::run_diagnostics(){
		size_t core_memory_size = m_inputs[0]->get_memory().size();

		if(core_memory_size > m_iteration_count)
			core_memory_size = m_iteration_count;

		// printf("Running diagnostics... %zu, #iter: %zu\n", core_memory_size, m_iteration_count);
		for(size_t i=0; i<core_memory_size; ++i){
			for(size_t j=0; j<m_inputs.size(); ++j){
				// printf("waiting index... %zu\n", core_memory_size-1-i);
				bool bit = m_inputs[j]->get_memory()[core_memory_size-1];
				// printf("setting bit %d\n", (int)bit);
				m_inputs[j]->set(bit);
			}
			// printf("Stabilizing... %.1f%%\n", (float)(i+1)/core_memory_size * 100.f);
			stabilize();
			// printf("Done\n");
		}
	}

	void System::update_interconnects(){
		m_interconnects.clear();
		size_t max_depth_m1 = max_depth() - 1;
		for(size_t i=0; i<max_depth_m1; ++i){
			auto layer = get(i);
			for(const auto& core: layer){
				for(const auto& forward_con: core->forward_connections()){
					Interconnect interconnect;
					interconnect.info = forward_con.second;
					interconnect.cores = std::pair<std::weak_ptr<Core>, std::weak_ptr<Core>>
						(core, forward_con.first);
					m_interconnects.push_back(interconnect);
				}
			}
		}
	}

	const std::vector<Interconnect>& System::get_interconnects() const{
		return m_interconnects;
	}

	Interconnect* System::get_interconnect(const std::weak_ptr<Core>& core1, 
			const std::weak_ptr<Core>& core2){
		Interconnect* interconnect = nullptr;
		auto it = std::find_if(m_interconnects.begin(), m_interconnects.end(), 
				[core1, core2](const Interconnect& interconnect){
					return (interconnect.cores.first.lock() == core1.lock() && 
								interconnect.cores.second.lock() == core2.lock()) ||
							(interconnect.info.speculative && 
								interconnect.cores.first.lock() == core2.lock() && 
								interconnect.cores.second.lock() == core1.lock());
				});
		if(it != m_interconnects.end())
			interconnect = &*it;
		return interconnect;
	}

	void System::stabilize(bool stm){
		size_t max_depth_ = max_depth();
		// printf("max depth: %zu\n", max_depth_);
		for(size_t i=0; i<max_depth_; ++i){
			auto cores = get(i);
			// printf("%zu: #cores: %zu\n", i, cores.size());
			for(auto& core: cores){
				// printf("propagating from core %zu, type: %d\n", core->id(), (int)core->type());
				bool tmp = core->propagate(stm);
				// printf("[%zu]: %d, %s\n", core->id(), (int)tmp, core->m_inputs.to_string().c_str());
			}	// printf("\n");
		}
	}

	System System::create(const raw_input_t& input, const raw_output_t& output) const{
		System system(1, 1);
		system.m_specialized_for = output;
		std::shared_ptr<Core> core, fcore, 
			s0 = std::static_pointer_cast<Core>(std::make_shared<AND>(m_policy.core_memory_size));
		system.m_cores.push_back(s0);

		for(size_t i=0; i<output.size(); ++i){
			core = std::static_pointer_cast<Core>(std::make_shared<Core>(m_policy.core_memory_size));
			core->set_name("o_" + std::to_string(i));
			fcore = core;
			if(!output[i]){
				fcore = std::static_pointer_cast<Core>
					(std::make_shared<NOT>(m_policy.core_memory_size));
				fcore->connect(core);
				// TODO: update system.m_interconnects
				system.m_cores.push_back(fcore);
			}
			s0->connect(fcore);
			// TODO: update system.m_interconnects
			system.m_outputs.push_back(core);
		}

		for(size_t i=0; i<input.size(); ++i){
			core = std::make_shared<Core>(m_policy.core_memory_size, true);
			core->set_name("i_" + std::to_string(i));
			core->set(input[i]);
			fcore = core;
			if(!input[i]){
				fcore = std::static_pointer_cast<Core>
					(std::make_shared<NOT>(m_policy.core_memory_size));
				core->connect(fcore);
				// TODO: update system.m_interconnects
				system.m_cores.push_back(fcore);
			}
			fcore->connect(s0);
			// TODO: update system.m_interconnects
			system.m_inputs.push_back(core);
		}
		system.stabilize();
		return system;
	}

	void System::compress(const System& system){
		printf("S: {\n");
		auto strs = to_strings();
		for(const auto& str: strs)
			printf("> %s\n", str.c_str());
		printf("}\nS': {\n");
		strs = system.to_strings();
		for(const auto& str: strs)
			printf("> %s\n", str.c_str());
		printf("}\n");

		size_t initial_core_count = m_inputs.size() + m_outputs.size() + m_cores.size();
		size_t merge_core_count = 0;
		size_t static_core_count = 0;
		size_t dynamic_core_count = 0;

		printf("#fcores: %zu\n", m_cores.size());
		auto fcores = system.m_cores;
		for(const auto& fcore: fcores){
			// printf(" dbg fcore type: %d\n", fcore->type());
			// printf(" dbg backward\n");
			m_cores.push_back(fcore);
			auto cores = fcore->backward_connections();
			for(auto& core_weak: cores){
				auto core = core_weak.first.lock();
				auto core_ = utils::find_core_by_name(core->name(), m_inputs);
				if(core->type() != CoreType::DEFAULT) continue;
				// printf("> name: %s\n", core->name().c_str());
				if(!core_){ /* printf("passing...\n"); */ continue; }
				// printf("! found: %s\n", core_->name().c_str());
				core->disconnect(fcore);
				// printf(" disconnecting... %zu, %zu\n", 
				// 		core->forward_connections().size(), fcore->backward_connections().size());
				core_->connect(fcore);
				// printf(" connecting %zu, %zu\n>>> %s\n", 
				// 		core_->forward_connections().size(), fcore->backward_connections().size(),
				// 		to_strings().back().c_str());
			}

			// printf(" dbg forward\n");
			cores = fcore->forward_connections();
			for(auto& core_weak: cores){
				auto core = core_weak.first.lock();
				auto core_ = utils::find_core_by_name(core->name(), m_outputs);
				if(core->type() != CoreType::DEFAULT) continue;
				// printf("> name: %s\n", core->name().c_str());
				if(!core_){ /* printf("passing...\n"); */ continue; }
				// printf("! found: %s\n", core_->name().c_str());
				fcore->disconnect(core);
				// printf(" disconnecting... %zu, %zu\n", 
				// 		fcore->forward_connections().size(), core->backward_connections().size());
				if(core_->backward_connections().empty()){
					// printf("[direct] %p\n", fcore.get());
					fcore->connect(core_);
				} else{
					// printf("[indirect]\n");
					std::shared_ptr<Core> s;
					size_t index = std::find_if(m_outputs.begin(), m_outputs.end(),
							[core_](const std::shared_ptr<Core>& core){
								return core == core_;
							}) - m_outputs.begin();
					if(system.m_specialized_for[index])
						s = std::static_pointer_cast<Core>
							(std::make_shared<Selector<1>>(m_policy.core_memory_size));
					else
						s = std::static_pointer_cast<Core>
							(std::make_shared<Selector<0>>(m_policy.core_memory_size));
					m_cores.push_back(s);

					auto back = core_->backward_connections().back().first.lock();
					back->disconnect(core_);
					back->connect(s);
					fcore->connect(s);
					s->connect(core_);
				}
				// printf(" connecting %zu, %zu\n>>> %s\n", 
				// 		fcore->forward_connections().size(), core_->backward_connections().size(),
				// 		to_strings().back().c_str());
			}
		}
		merge_core_count = m_inputs.size() + m_outputs.size() + m_cores.size();
		printf(" === DONE ===\n");
		printf("--> %s\n", to_strings().back().c_str());
		printf("[merged] #fcores: %zu\n", m_cores.size());

		for(size_t i=0; i<m_inputs.size(); ++i){
			// printf("%zu: %s (before)\n", i, m_inputs[i]->get_memory().to_string().c_str());
			m_inputs[i]->set(system.m_inputs[i]->get_memory()[0]);
		}
		// printf("stabilizing...\n");
		stabilize();
		// for(size_t i=0; i<m_inputs.size(); ++i){
		// 	printf("%zu: %s (after)\n", i, m_inputs[i]->get_memory().to_string().c_str());
		// }

		// Static Compression
		const char* types[] = { "io", "s<0>", "s<1>", "not" };
		for(size_t i=0; i<m_cores.size(); ++i){
			auto& fcore1 = m_cores[i];
			for(size_t j=i+1; j<m_cores.size(); ++j){
				auto& fcore2 = m_cores[j];
				// printf("ids: %zu %zu\n", fcore1->id(), fcore2->id());
				// printf("types: %s %s\n", types[fcore1->type()], types[fcore2->type()]);
				// printf("hashes: %zu %zu\n", fcore1->hash(), fcore2->hash());
				if(fcore1->hash() == fcore2->hash() && fcore1->type() == fcore2->type()){
					// printf("similar sub-structure detected...\n");
					auto back_cons = fcore2->backward_connections();
					for(auto& fcore_weak: back_cons){
						auto fcore = fcore_weak.first.lock();
						fcore->disconnect(fcore2);
					}
					auto forward_cons = fcore2->forward_connections();
					for(auto& fcore_weak: forward_cons){
						auto fcore = fcore_weak.first.lock();
						fcore2->disconnect(fcore);
						fcore1->connect(fcore);
					}
					// printf("cleaning up...\n");
					m_cores.erase(m_cores.begin()+(j--));
				}	// printf("---\n");
			}	// printf("===========\n");
		}
		static_core_count = m_inputs.size() + m_outputs.size() + m_cores.size();
		printf("[static] #fcores: %zu\n", m_cores.size());

		// Dynamic Compression
		size_t max_depth_m1 = max_depth() - 1;
		size_t radius = max_depth_m1 - 1;
		std::deque<std::vector<std::shared_ptr<Core>>> layers;

		if(radius > m_policy.compression_radius)
			radius = m_policy.compression_radius;

		++m_iteration_count;
		run_diagnostics();
		// printf("after diagnostics:\n");
		// for(const auto& core: m_inputs)
		// 	printf("%s(%zu) has memory %s\n", 
		// 			types[core->type()], core->id(), core->get_memory().to_string().c_str());
		// printf("---\n");
		// for(const auto& core: m_outputs)
		// 	printf("%s(%zu) has memory %s\n", 
		// 			types[core->type()], core->id(), core->get_memory().to_string().c_str());
		for(size_t i=0; i<radius+1; ++i){
			layers.push_back(get(max_depth_m1 - 1 - i));
		}
		// printf("preloaded depth range: [%zu, %zu]\n", max_depth_m1-1, max_depth_m1-1-radius);
		for(size_t i=1; i<max_depth_m1; ++i){
			auto& layer = layers[0]; // layers.front();
			// printf("-> [%zu] layer depth: %zu, size: %zu\n", 
			// 		0ul, layer.front()->depth(), layer.size());
			for(size_t j=0; j<layers.size()-1; ++j){
				auto& slayer = layers[layers.size() - 1 - j];
				// printf("--> [%zu] slayer depth: %zu, size: %zu\n", 
				// 		layers.size()-1-j, slayer.front()->depth(), slayer.size());

				// layer - slayer
				for(size_t p=0; p<layer.size(); ++p){
					auto core = layer[p];
					if(core->forward_connections().empty()) continue;
					// printf(" > main core: %s(%zu)\n", types[core->type()], core->id());
					for(size_t q=0; q<slayer.size(); ++q){
						auto score = slayer[q];
						if(score->forward_connections().empty()) continue;
						// get short-term-memory similarity %
						// if similar enough -> make_interconnect or replace
						auto mem1 = core->get_memory();
						auto mem2 = score->get_memory();
						mem1 = mem1.substream(mem1.begin(), m_iteration_count);
						mem2 = mem2.substream(mem2.begin(), m_iteration_count);
						// printf("%s[%s(%zu)] vs %s[%s(%zu)]\n",
						// 	mem1.to_string().c_str(), types[core->type()], core->id(),
						// 	mem2.to_string().c_str(), types[score->type()], score->id());
						if(mem1 == mem2){ // stm similarity = 100%
							// printf("replacing, %s = %s\n", 
							// 		core->get_memory().to_string().c_str(), 
							// 		score->get_memory().to_string().c_str()); 
							core->replace_with(score);
							// printf("done\n");
							--p;
							break;
						}
					}
				}
			}
			layers.pop_front();
			if(max_depth_m1 >= 1 + radius + i){
				// printf("poping front, adding depth %zu\n", max_depth_m1-1-radius-i);
				layers.push_back(get(max_depth_m1 - 1 - radius - i));
			}
		}
		for(auto it=m_cores.begin(); it!=m_cores.end(); ){
			auto core = it->get();
			if(core->forward_connections().empty()){
				it = m_cores.erase(it);
			} else{
				++it;
			}
		}
		// update_interconnects();
		dynamic_core_count = m_inputs.size() + m_outputs.size() + m_cores.size();
		printf("[dynamic] #fcores: %zu\n", m_cores.size());
		printf("<--- Statistics --->\n");
		printf("Statically compressed(%%):  %.1f\n",
				(float)(merge_core_count-static_core_count)/merge_core_count * 100);
		printf("Dynamically compressed(%%): %.1f\n",
				(float)(static_core_count-dynamic_core_count)/static_core_count * 100);
		printf("Effectively compressed(%%): %.1f\n",
				(float)(merge_core_count-dynamic_core_count)/merge_core_count * 100);
		printf("Compressed(%%):             %.1f\n",
				((float)initial_core_count-(float)dynamic_core_count)/initial_core_count * 100);
		printf("<--- ---------- --->\n\n");
	}

	void System::to_string(std::string& expr, const std::weak_ptr<Core>& core_){
		std::shared_ptr<Core> core = core_.lock();
		if(!core){
			expr = "?";
			// fprintf(stderr, "null core\n");
			return;
		}
		else if(core->backward_connections().empty()){
			expr = core->name();
			if(expr.empty()){
				expr = "c_" + std::to_string(core->id());
				// fprintf(stderr, "nameless core that has no backward connections\n");
			}
			return;
		}

		std::string sub_expr, sign;
		auto cores = core->backward_connections();

		to_string(sub_expr, cores[0].first);
		if(core->type() == CoreType::NEGATOR)
			sign = "~";
		expr = sign + "(" + sub_expr + ")";

		if(core->type() == CoreType::SELECTOR_0)
			sign = " & ";
		else if(core->type() == CoreType::SELECTOR_1)
			sign = " | ";

		for(size_t i=1; i<cores.size(); ++i){
			to_string(sub_expr, cores[i].first);
			expr += sign + "(" + sub_expr + ")";
		}
	}

	void System::from_string(const std::string& expr){
		auto core = std::make_shared<Core>();
		core->set_name("o_"+ std::to_string(m_outputs.size()));
		m_outputs.push_back(core);
		// TODO
	}
}
