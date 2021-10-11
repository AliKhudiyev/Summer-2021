#include "io.h"
#include <cmath>
#include <memory>
#include <random>
#include <algorithm>

#define IO_MAX_CONNECTIONS 5

namespace alc{
	void IO::add(const raw_input_t& input, const raw_output_t& output){
		BitStream bs(m_input_count + m_output_count);
		for(size_t i=0; i<m_input_count; ++i)
			bs[i] = input[i];
		for(size_t i=m_input_count; i<m_input_count+m_output_count; ++i)
			bs[i] = output[i-m_input_count];
		m_table.push_back(bs);
	}

	void IO::remove_input(const raw_input_t& input){
	}

	void IO::remove_output(const raw_output_t& output){
	}

	raw_output_t IO::eval(const raw_input_t& input) const{
		return generate(input);
	}

	std::string IO::eval(const std::string& input) const{
		raw_input_t input_(input);
		return eval(input_).to_string();
	}

	void IO::eval(size_t set_count, bool repetitions){
		size_t count = pow(2, m_input_count);
		if(count > set_count)
			count = set_count;

		raw_input_t input(m_input_count, 0);
		for(size_t i=0; i<count; ++i){
			add(input, eval(input));
			for(size_t j=input.size(); j; --j){
				if(!input[j-1]){
					input[j-1] = 1;
					break;
				} else{
					input[j-1] = 0;
				}
			}
		}
	}

	raw_output_t IO::generate(const raw_input_t& input) const{
		return m_system.predict(input);
	}

	std::string IO::generate(const std::string& input) const{
		raw_input_t input_(input);
		return generate(input_).to_string();
	}

	std::pair<raw_input_t, raw_output_t> IO::generate() const{
		std::pair<raw_input_t, raw_output_t> output;
		output.first.reset(m_input_count);

		for(size_t i=0; i<m_input_count; ++i)
			output.first[i] = rand() % 2;
		output.second = generate(output.first);

		return output;
	}

	void IO::generate(size_t set_count, size_t output_count, bool repetitions){
		size_t input_count = static_cast<size_t>(ceil(log2(static_cast<float>(set_count))));
		generate(input_count, output_count, set_count, 1, 5, 0.5f);
	}

	void IO::generate(size_t input_count, size_t output_count, size_t set_count, size_t min_or_count, size_t max_or_count, float not_freq, bool repetitions){
		size_t and_count = output_count;
		size_t not_count = input_count * not_freq;
		size_t or_count = and_count * (min_or_count + max_or_count) / 2;

		// Reinitilizing
		m_input_count = input_count;
		m_output_count = output_count;
		m_table.resize(set_count);
		m_system.reset(m_system.m_policy, m_system.m_options);

		// Random system generation
		printf("Random system generation...\n");
		for(size_t i=0; i<input_count; ++i){
			auto core = std::make_shared<Core>();
			core->set_name("i_" + std::to_string(i));
			m_system.m_inputs.push_back(core);
		}
		for(size_t i=0; i<output_count; ++i){
			auto core = std::make_shared<Core>();
			core->set_name("o_" + std::to_string(i));
			m_system.m_outputs.push_back(core);
		}

		printf("Generating functional cores... and(%zu), or(%zu), not(%zu)\n",
				and_count, or_count, not_count);
		std::vector<std::shared_ptr<Core>> ands;
		for(size_t i=0; i<and_count; ++i){
			auto fcore = std::static_pointer_cast<Core>(std::make_shared<AND>());
			fcore->connect(m_system.m_outputs[i]);
			m_system.m_cores.push_back(fcore);
			ands.push_back(fcore);
		}
		
		std::vector<std::shared_ptr<Core>> ors;
		for(size_t i=0; i<or_count; ++i){
			auto fcore = std::static_pointer_cast<Core>(std::make_shared<OR>());
			// m_system.m_cores.push_back(fcore);
			ors.push_back(fcore);
		}

		std::vector<std::shared_ptr<Core>> nots;
		for(size_t i=0; i<not_count; ++i){
			auto fcore = std::static_pointer_cast<Core>(std::make_shared<NOT>());
			// m_system.m_inputs[i]->connect(fcore);
			// m_system.m_cores.push_back(fcore);
			nots.push_back(fcore);
		}

		printf("Configuring interconnects...\n");
		size_t n_con = std::min(input_count+1, (size_t)IO_MAX_CONNECTIONS+1);
		size_t index_ = 0;

		for(size_t i=0, index; i<and_count; ++i){
			// std::random_shuffle(ors.begin()+index, ors.end());
			
			size_t count = min_or_count + rand()%(max_or_count-min_or_count+1);
			std::shared_ptr<Core> fcore;
			for(size_t j=0; j<count; ++j){
				fcore = ors[(index + j) % ors.size()];
				if(!fcore->backward_connections().empty()) continue;
				m_system.m_cores.push_back(fcore);
				// printf(">> OR\n");

				size_t n = rand() % n_con;
				size_t m = 0;
				
				if(n < IO_MAX_CONNECTIONS){
					m = rand() % (IO_MAX_CONNECTIONS - n);
					if(nots.empty())
						n = m;
				}

				if(n + m < 2){
					int tmp = rand()%3;
					if(!tmp)
						n = 2;
					else if(tmp == 1 && nots.size() > 1)
						m = 2;
					else
						n = m = 1;
				}

				// printf("choosing random cores... %zu, %zu\n", n, m);
				auto cores = choose(m_system.m_inputs, n);
				auto ncores = choose(nots, m);

				std::string tmp;
				// printf("||| showing connections...\n");
				for(auto& core: cores){
					// System::to_string(tmp, core);
					if(core->connected(fcore)){
						// printf("cannot connect to %s\n", tmp.c_str());
						continue;
					}
					core->connect(fcore);
					m_system.m_interconnects.push_back({
							{ 1.f, false }, 
							std::pair<std::weak_ptr<Core>, std::weak_ptr<Core>>(core, fcore)
					});
					// printf("connecting %s, depth: %zu\n", tmp.c_str(), fcore->depth());
				}
				// printf("||| showing connections 2...\n");
				for(auto& core: ncores){
					// System::to_string(tmp, core);
					if(core->connected(fcore)/* ||
						(std::find(cores.begin(), cores.end(), 
							core->backward_connections()[0].first.lock()) != cores.end())*/){
						// printf("cannot connect to %s\n", tmp.c_str());
						continue;
					}
					if(core->backward_connections().empty()){
						// printf("making connection to an input core...\n");
						m_system.m_inputs[(index_++)%m_system.m_inputs.size()]->connect(core);
						m_system.m_cores.push_back(core);
					}
					core->connect(fcore);
					m_system.m_interconnects.push_back({
							{ 1.f, false }, 
							std::pair<std::weak_ptr<Core>, std::weak_ptr<Core>>(core, fcore)
					});
					// printf("connecting %s, depth: %zu\n", tmp.c_str(), fcore->depth());
				}
				fcore->connect(ands[i]);
				m_system.m_interconnects.push_back({
						{ 1.f, false }, 
						std::pair<std::weak_ptr<Core>, std::weak_ptr<Core>>(fcore, ands[i])
				});
				// printf("(%zu, %zu) connecting to AND, depth: %zu\n", i, j, ands[i]->depth());
				// printf("output depth: %zu\n", m_system.m_outputs[i]->depth());
			}
			// printf("> Done\n");
			index += count;
		}
		
		// Clean-up
		printf("Cleaning up...\n");
		// m_system.cleanup();

		// Input data generation
		printf("Generating IO table...\n");
		for(size_t i=0; i<set_count; ++i){
			BitStream bs(input_count+output_count);
			for(size_t j=0; j<input_count; ++j)
				bs[j] = rand() % 2;
			m_table[i] = bs;
			printf("%s\n", bs.to_string().c_str());
		}

		printf("Finalizing... input cores: %zu, output cores: %zu, functional cores: %zu\n",
				m_system.m_inputs.size(), m_system.m_outputs.size(), m_system.m_cores.size());
		
		auto exprs = m_system.to_strings();
		for(const auto& expr: exprs)
			printf("%s\n", expr.c_str());

		m_system.predict(*this);

		for(const auto& bs: m_table)
			printf("%s\n", bs.to_string().c_str());
	}

	size_t IO::input_count() const{
		return m_input_count;
	}

	size_t IO::output_count() const{
		return m_output_count;
	}

	size_t IO::size() const{
		return m_table.size();
	}

	raw_input_t IO::input_at(size_t index) const{
		const auto& bs = m_table[index];
		return bs.substream(bs.cbegin(), bs.cbegin()+m_input_count);
	}

	raw_output_t IO::output_at(size_t index) const{
		const auto& bs = m_table[index];
		return bs.substream(bs.cbegin()+m_input_count, bs.cend());
	}

	System IO::system() const{
		return m_system;
	}

	void IO::input_at(size_t index, const raw_input_t& input){
		auto& bs = m_table[index];
		for(size_t i=0; i<m_input_count; ++i)
			bs[i] = input[i];
	}

	void IO::output_at(size_t index, const raw_output_t& output){
		auto& bs = m_table[index];
		size_t sz = m_input_count + m_output_count;
		for(size_t i=m_input_count; i<sz; ++i)
			bs[i] = output[i-m_input_count];
	}

	void IO::save(const char* file_path) const{
		FILE* file = fopen(file_path, "w");
		if(!file)
			exit(1);

		std::string str;
		for(size_t i=0; i<m_input_count; ++i)
			fprintf(file, "i_%s ", std::to_string(i).c_str());
		for(size_t i=0; i<m_output_count; ++i)
			fprintf(file, "o_%s ", std::to_string(i).c_str());

		for(const auto& bs: m_table){
			for(size_t i=0; i<bs.size(); ++i){
				fprintf(file, "%d ", (int)bs[i]);
			}	fprintf(file, "\n");
		}

		fclose(file);
	}

	bool IO::load(const char* file_path){
		FILE* file = fopen(file_path, "r");
		if(!file)
			return !LOAD_OK;

		char buf[64];
		char bit;
		m_input_count = m_output_count = 0;

		while(1){
			BitStream bs(m_input_count+m_output_count);
			m_table.push_back(bs);

			for(size_t i=0; i<m_input_count; ++i){
				fscanf(file, "%c", &bit);
				m_table.back()[i] = bit;
			}
			for(size_t i=0; i<m_output_count; ++i){
				fscanf(file, "%c", &bit);
				m_table.back()[i] = bit;
			}

			if(!fscanf(file, "%s", buf))
				break;
		}

		fclose(file);
		return LOAD_OK;
	}

	void IO::shuffle(){
		std::random_shuffle(m_table.begin(), m_table.end());
	}

	BitStream& IO::operator[](size_t index){
		return m_table[index];
	}
}
