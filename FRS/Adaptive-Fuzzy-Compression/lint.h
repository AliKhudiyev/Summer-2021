
#pragma once

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <ctime>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <array>


using Core 			= char;
// using Interconnect 	= std::pair<std::pair<const Core*, const Core*>, size_t>;
using info_t = std::map<size_t /* time_diff */, size_t /* support */>;

struct Interconnect{
	static size_t interconnect_count;

	size_t id;
	const Core* const prev_core;
	const Core* const next_core;
	info_t info;

	Interconnect(const Core* const pcore, const Core* const ncore):
		id(++interconnect_count), prev_core(pcore), next_core(ncore) {}

	Interconnect(const Core* const pcore, const Core* const ncore, const info_t& info_):
		id(++interconnect_count), prev_core(pcore), next_core(ncore), info(info_) {}
		// Interconnect(pcore, ncore), info(info_) {}

	~Interconnect(){}

	inline bool equals(const Core* const pcore, const Core* const ncore) const{
		return prev_core == pcore && next_core == ncore;
	}

	inline bool equals(const Interconnect& interconnect) const{
		return equals(interconnect.prev_core, interconnect.next_core);
	}

	bool operator<(const Interconnect& interconnect) const{
		return id < interconnect.id;
	}

	bool operator>(const Interconnect& interconnect) const{
		return id > interconnect.id;
	}
};

size_t Interconnect::interconnect_count = 0;

class Graph{
	private:
	std::set<Core> m_cores;
	std::set<Interconnect> m_interconnects;

	public:
	Graph() = default;
	~Graph(){}

	inline void add_core(const Core& core){
		m_cores.insert(core);
		// printf(" dbg added core: %c\n", *m_cores.find(core));
	}

	inline void remove_core(const Core& core){
		m_cores.erase(core);
	}

	inline size_t core_count() const { return m_cores.size(); }
	inline size_t interconnect_count() const { return m_interconnects.size(); }

	void connect(const Core& core1, const Core& core2, size_t time_diff, size_t support=1){
		auto it1 = m_cores.find(core1);
		auto it2 = m_cores.find(core2);
		
		if(it1 == m_cores.end() || it2 == m_cores.end()){
			// printf("\n dbg impossible connection: %c - %c\n", core1, core2);
			return ;
		}

		auto it = std::find_if(m_interconnects.begin(), m_interconnects.end(), 
				[it1, it2](const Interconnect& interconnect){
					return interconnect.equals((const Core* const)&*it1, (const Core* const)&*it2);
				});

		if(it == m_interconnects.end()){
			Interconnect tmp ( 
				(const Core* const)&*it1, 
				(const Core* const)&*it2
			);
			tmp.info[time_diff] = support;
			m_interconnects.insert(tmp);
			return ;
		}
		
		((Interconnect*)&*it)->info[time_diff] += support;
	}

	size_t support(const Core& core1, const Core& core2, size_t time_diff) const{
		auto it1 = std::find(m_cores.cbegin(), m_cores.cend(), core1);
		auto it2 = std::find(m_cores.cbegin(), m_cores.cend(), core2);
		auto it = std::find_if(m_interconnects.cbegin(), m_interconnects.cend(), 
				[it1, it2](const Interconnect& interconnect){
					return interconnect.equals((const Core* const)&*it1, (const Core* const)&*it2);
				});
		return (it == m_interconnects.cend() ? 0 : it->info.at(time_diff));
	}

	std::map<Core, size_t> support(const Core& core, size_t time_diff) const{
		std::map<Core, size_t> mp;
		auto it1 = std::find(m_cores.cbegin(), m_cores.cend(), core);
		for(auto it=m_interconnects.cbegin(); it!=m_interconnects.cend(); ++it){
			info_t::const_iterator it_info;
			if(it->prev_core == (const Core* const)&*it1 && 
			  (it_info=it->info.find(time_diff)) != it->info.cend()){
				mp[*(it->next_core)] = it_info->second;
			}
		}
		return mp;
	}

};

class LINT{
	private:
	Graph m_graph;

	private:
	static bool is_stop_char(char c){
		return c == ' ' || c == ',' || c == '.' || c == '!' || c == '?' || c == ':' ||
			c == ';' || c == '\'' || c == '"' || c == '(' || c == ')' || c == '-' ||
			c == '[' || c == ']' || c == '<' || c == '>' || c == '\\' || c == '/' ||
			c == '@' || c == '#' || c == '$' || c == '%' || c == '^' || c == '&' ||
			c == '_' || c == '=' || c == '+' || c == '*' || c == '`' || c == '~' || c == 0;
	}

	public:
	LINT() = default;
	~LINT(){}

	void train(const char* file_path, size_t max_time_sec=-1){
		FILE* file = fopen(file_path, "r");

		if(!file){
			printf("File could not be opened! %s\n", file_path);
			return ;
		}

		const int mem_size = 3;
		char mem[mem_size] = { 0 };
		size_t char_count = 0;
		time_t start_ts = time(NULL), curr_ts = start_ts;

		while((mem[0]=fgetc(file)) != EOF && curr_ts - start_ts <= max_time_sec){		
			if(mem[0] < 97 || mem[0] > 122)
				continue;
			m_graph.add_core(mem[0]);
			for(int i=1; i<mem_size; ++i)
				m_graph.connect(mem[0], mem[i], i); // mem_size-i);
			for(int i=mem_size-1; i>0; --i){
				mem[i]=mem[i-1];
			}
			// for(int i=mem_size-1; i>0; m_graph.connect(mem[0], mem[i], mem_size-i), mem[i]=mem[--i]);
			time(&curr_ts);
		}

		fclose(file);

		printf("\n# cores: %zu\n# interconnects: %zu\n", 
				m_graph.core_count(), m_graph.interconnect_count());
	}

	char predict_char(const std::string& chars) const{
		char c_pred = 0;
		std::map<char, size_t> mp;

		for(size_t i=0; i<chars.size(); ++i){
			char c = chars[i];
			fprintf(stderr, "\nchar: %c\n", c);
			std::map<char, size_t> tmp = m_graph.support(c, chars.size()-i);
			for(auto it=tmp.cbegin(); it!=tmp.cend(); ++it){
				fprintf(stderr, " > %c : %zu\n", it->first, it->second);
				mp[it->first] += it->second;
			}
		}

		size_t max = 0;
		for(auto it=mp.cbegin(); it!=mp.cend(); ++it){
			// printf(" dbg %c: %zu\n", it->first, it->second);
			if(max < it->second){
				max = it->second;
				c_pred = it->first;
			}
		}

		return c_pred;
	}

	std::string predict_word(const std::string& chars) const{
		std::string word_pred;
		char c = 0;
		size_t size = 0;
		while(!is_stop_char((c=predict_char(chars))) && size++ < 20)
			word_pred += c;
		return word_pred;
	}

	void test(const std::string& file_path, size_t max_time_sec=0){
		;
	}
};

