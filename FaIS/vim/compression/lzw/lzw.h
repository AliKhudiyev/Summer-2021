
#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include "bit_stream.h"


namespace lzw{
	using table_t = std::vector<std::string>;
	using compressed_data_t = std::vector<size_t>;

	compressed_data_t compress(const std::string& data){
		compressed_data_t out;
		table_t table(128);

		// Initializing the table
		printf("Initializing the table...\n");
		for(unsigned char c=0; c<128; ++c)
			table[c] = std::string(1, c);

		// Compressing
		printf("Compressing...\n");
		std::string curr, tmp;
		for(size_t i=0; i<data.size(); ++i){
			const char& c = data[i];
			tmp = curr + c;
			auto it = std::find(table.cbegin(), table.cend(), tmp);

			if(it == table.cend()){
				out.push_back(std::find(table.cbegin(), table.cend(), curr) - table.cbegin());
				table.push_back(tmp);
				curr = std::string(1, c);
			} else{
				curr = tmp;
			}
		}
		out.push_back(std::find(table.cbegin(), table.cend(), curr) - table.cbegin());
		
		char null = 0;
		for(size_t i=0; i<table.size(); ++i){
			const char* str = &null;
			if(table[i][0] >= 32 && table[i][0] <= 127)
				str = table[i].c_str();
			printf("[%s]: %zu\n", str, i);
		}

		for(const auto& index: out)
			printf("%zu ", index);
		printf("\n");

		return out;
	}

	std::string decompress(const compressed_data_t& data){
		std::string out;
		table_t table(128);

		// Initializing the table
		for(unsigned char c=0; c<128; ++c)
			table[c] = std::string(1, c);

		// Decompressing
		size_t index;
		std::string curr = table[data[0]], tmp;
		out = curr;
		for(size_t i=1; i<data.size(); ++i){
			index = data[i];
			if(index >= table.size()){
				tmp += tmp[tmp.size()-1];
			} else{
				tmp = curr + table[index];
				// curr = table[index];
			}
			out += tmp;
			table.push_back(tmp);
		}

		return out;
	}
}

