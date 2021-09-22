
#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <queue>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include "bit_stream.h"


namespace huffman{
	using compressed_data_t = std::pair<std::vector<unsigned char> /* data */, size_t /* offset */>;
	using encoded_data_t = compressed_data_t;
	using table_t = std::map<char, std::vector<bool>>;

	struct Node{
		char c;
		size_t freq;
		Node *left, *right;
		Node *top;

		Node(char c_=0, size_t freq_=0, Node* left_=nullptr, Node* right_=nullptr, Node* top_=nullptr): 
			c(c_), freq(freq_), left(left_), right(right_), top(top_) {}
		~Node(){
			if(!left)
				delete left;
			if(!right)
				delete right;
		}
	};

	std::pair<compressed_data_t, table_t> compress(const std::string& data){
		using pair_t = std::pair<char, size_t>;

		compressed_data_t out;
		std::vector<pair_t> vp;
		std::map<char, size_t> mp;

		// printf("Initializing...\n");
		// Obtaining frequencies for each occuring character
		for(const auto& c: data)
			mp[c] += 1;

		// Sorting characters according to the frequencies
		vp.assign(mp.cbegin(), mp.cend());
		std::sort(vp.begin(), vp.end(), 
				[](const pair_t& p1, const pair_t& p2){
					return p1.second > p2.second;
				});

		// Initializing the unit nodes
		std::vector<Node*> nodes;
		for(const auto& p: vp)
			nodes.push_back(new Node(p.first, p.second));
		std::vector<Node*> nodes_copy = nodes;

		// Obtaining the Huffman Tree
		// printf("Obtaining Huffman Tree...\n");
		while(nodes.size() > 1){
			Node* node1 = nodes.back();
			nodes.pop_back();

			Node* node2 = nodes.back();
			nodes.pop_back();

			Node* node = new Node(0, node2->freq + node1->freq, node2, node1);
			node1->top = node2->top = node;

			for(size_t i=0; i<nodes.size(); ++i){
				if(node->freq > nodes[i]->freq){
					nodes.insert(nodes.begin()+i, node);
					break;
				}
				if(i+1 == nodes.size()){
					nodes.push_back(node);
					break;
				}
			}
			if(nodes.empty())
				nodes.push_back(node);
		}

		// Obtaining the lookup table
		// printf("Obtaining lookup table...\n");
		table_t table;

		for(auto node: nodes_copy){
			std::vector<bool> bits;
			auto curr = node;
			while(curr->top){
				bits.push_back(curr->top->right == curr);
				curr = curr->top;
			}
			table[node->c] = std::vector<bool>(bits.crbegin(), bits.crend());
		}

		// Compressing the data
		// printf("Compressing the data...\n");
		BitStream bs;
		for(const auto& c: data){
			for(const bool& bit: table[c])
				bs.push(bit, bs.end());
		}
		
		out.first = bs.buffer();
		out.second = bs.gap();
		
		return std::make_pair(out, table);
	}

	std::string decompress(const compressed_data_t& data, const table_t& table){
		std::string out;
		std::vector<bool> bits;
		char c = 0;

		BitStream bs;
		bs.rassign<unsigned char>(data.first.data(), 8*data.first.size()-data.second, data.second);

		auto it = bs.cbegin();
		while(it != bs.cend()){
			std::vector<bool> tmp;
			while(it != bs.cend()){
				tmp.push_back(*it);
				++it;
				auto it_table = std::find_if(table.cbegin(), table.cend(),
						[tmp](const std::pair<char, std::vector<bool>>& p){
							return p.first && tmp == p.second;
						});
				if(it_table != table.cend()){
					out += it_table->first;
					break;
				}
			}
		}

		return out;
	}
}

