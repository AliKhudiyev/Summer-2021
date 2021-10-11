
#pragma once

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <vector>
#include <string>
#include <string.h>
#include <set>
#include <map>
#include <array>
#include <memory>


using info_t = std::map<size_t /* time_diff */, size_t /* support */>;


struct Core;

struct Interconnect{
	const Core* const core;
	info_t info;

	Interconnect(const Core* const core_=nullptr): core(core_) {}
	~Interconnect(){}

	// not recommended
	inline bool equals(char c) const;
	
	// recommended
	inline bool equals(const Core* const core_) const;
	inline bool equals(const Interconnect& interconnect) const;

};

struct Core{
	char c;
	std::vector<Interconnect> nexts;

	Core(char c_=0): c(c_) {}
};

// not recommended
inline bool Interconnect::equals(char c) const { return core->c == c; }

// recommended
inline bool Interconnect::equals(const Core* const core_) const { return core == core_; }
inline bool Interconnect::equals(const Interconnect& interconnect) const { 
	return equals(interconnect.core); 
}

class LINT2{
	static std::vector<Core> cores;

	public:
	LINT2() = default;
	~LINT2(){}

	const Core* find_char(char c) const{
		auto it = std::find_if(cores.cbegin(), cores.cend(),
				[c](const Core& core){
					return core.c == c;
				});
		return it == cores.cend() ? nullptr : (const Core*)&*it;
	}

	const Interconnect* find_interconnect(char prev_c, char next_c) const{
		const Core* prev_core = find_char(prev_c);
		if(!prev_core)
			return nullptr;

		auto it = std::find_if(prev_core->nexts.cbegin(), prev_core->nexts.cend(),
				[next_c](const Interconnect& interconnect){
					return interconnect.core->c == next_c;
				});

		if(it == prev_core->nexts.cend())
			return nullptr;
		return (const Interconnect*)&*it;
	}

	std::map<char, size_t> support(char c, size_t time_diff) const{
		std::map<char, size_t> mp;
		const Core* core = find_char(c);

		if(!core)
			return mp;

		const auto& nexts = core->nexts;
		for(const auto& interconnect: nexts){
			auto it = interconnect.info.find(time_diff);
			if(it == interconnect.info.cend())	continue;
			mp[interconnect.core->c] += interconnect.info.at(time_diff);
		}

		return mp;
	}

	void train(const char* file_path, size_t max_time_sec=-1){
		FILE* file = fopen(file_path, "r");

		if(!file){
			printf("File could not be opened! %s\n", file_path);
			return ;
		}
	
		cores.clear();
		cores.reserve(27);
		if(cores.capacity() < 27){
			fprintf(stderr, "Not enough capacity!\n");
			exit(1);
		}
		cores.push_back(Core());
		Core* ptr_null_core = &cores.front();

		const int size = 10;
		char mem[size] = { 0 };
		Core* core_buf[size] = { ptr_null_core };
		time_t start_ts = time(NULL), curr_ts = start_ts;
		
		for(int i=0; i<size; core_buf[i++] = ptr_null_core);

		while((mem[0]=fgetc(file)) != EOF && curr_ts-start_ts <= max_time_sec){
			/* adding a new core if necessary */
			std::vector<Core>::const_iterator it;
			if(mem[0] >= 97 && mem[0] <= 122){
				it = std::find_if(cores.cbegin(), cores.cend(),
						[mem](const Core& core){
							return core.c == mem[0];
						});
				if(it == cores.cend()){
					cores.push_back(Core(mem[0]));
					it = cores.cend() - 1;
				}
			}
			else{
				it = cores.cbegin();
			}
			core_buf[0] = (Core*)&*it;

			/* updating the interconnects */
			for(int i=1; i<size; ++i){
				Core* current_core = core_buf[i];
				if(!current_core || (!current_core->c && current_core != ptr_null_core)){
					printf("wth!\n");
					exit(1);
				}

				/* finding relevant (next) interconnect */
				auto next = std::find_if(current_core->nexts.cbegin(), current_core->nexts.cend(), 
						[it](const Interconnect& interconnect){
							return interconnect.core == (const Core* const)&*it;
						});
				if(next == current_core->nexts.cend()){
					const Core* const tc = (const Core* const)&*it;
					Interconnect tmp((const Core* const)&*it);
					tmp.info[i] = 0;
					current_core->nexts.push_back(tmp);
					next = current_core->nexts.cend() - 1;
				}
				++((Interconnect*)&*next)->info[i];
			}

			/* shifting the memory and core buffers */
			for(int i=size-1; i>0; --i){
				mem[i] = mem[i-1];
				core_buf[i] = core_buf[i-1];
			}

			/* updating the time */
			time(&curr_ts);
		}

		fclose(file);
		
		printf("#cores: %zu\n", cores.size());
	}

	char predict_char(const std::string& chars) const{
		char c_pred = 0;
		std::map<char, size_t> mp = support(0, chars.size()+1);

		auto print_map = [](const std::map<char, size_t>& mp) -> void{
			for(auto it=mp.cbegin(); it!=mp.cend(); ++it){
				printf("%c: %zu\n", it->first, it->second);
			}
		};

		print_map(mp);
		for(size_t i=0; i<chars.size(); ++i){
			printf("\tCHAR: %c\n", chars[i]);
			std::map<char, size_t> tmp = support(chars[i], chars.size()-i);
			print_map(tmp);
			for(auto it=tmp.cbegin(); it!=tmp.cend(); ++it){
				if(mp.find(it->first) == mp.cend())	continue;
				mp[it->first] += it->second;
			}
		}

		size_t max = 0;
		for(auto it=mp.cbegin(); it!=mp.cend(); ++it){
			fprintf(stderr, "> %c: %zu\n", it->first, it->second);
			if(max < it->second && it->first){
				max = it->second;
				c_pred = it->first;
			}
		}

		return c_pred;
	}

	std::string predict_word(const std::string& chars) const{
		std::string word_pred = chars;
		char c = 0;
		while((c=predict_char(chars)))
			word_pred += c;
		return word_pred;
	}

};

std::vector<Core> LINT2::cores;

