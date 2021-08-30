
#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <queue>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>


#define MASK(n)	(1 << (n))
#define MASK_(n, x)	((x) << (n))

// returns `n`th bit of `c` indexing from the right, e.g. 0101 -> BIT_(0) = BIT_(2) = 1
#define BIT_(n, c)	(((c) << (8*sizeof(char)-1-(n))) >> (8*sizeof(char)-1))

// shifts `c` n bits to left and appends the first n bits of `c_`
#define SHIFT(c, c_, n)	(((c) << (n)) | ((c_) >> (8*sizeof(char)-(n))))

// returns `n`th bit of `ptr_c` stream indexing from the right,
// e.g. 1111 1111 0000 0000 -> BIT(7) = 0, BIT(8) = 1
#define BIT(n, ptr_c)	BIT_((n) % sizeof(char), (ptr_c)[(n) / sizeof(char)])

namespace huffman{
	using compressed_data_t = std::pair<std::vector<char> /* data */, size_t /* offset */>;
	using encoded_data_t = compressed_data_t;

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

	std::pair<std::vector<char>, size_t> convert_to_bit_stream(const std::vector<bool>& bits){
		size_t size = bits.size() / 8*sizeof(char) + (bool)(bits.size() % (8*sizeof(char)));
		size_t offset = 8*sizeof(char) - bits.size() % (8*sizeof(char));
		if(offset == 8 && size) offset = 0;

		std::vector<char> out(size);

		for(size_t i=bits.size(), index=size; i>0; --index){
			unsigned char tmp = 0;
			for(int j=0; j<8*sizeof(char) && i>0; ++j, --i){
				tmp = (tmp | (bits[i-1] << (8*sizeof(char)-1)));
				if(j != 8*sizeof(char)-1) tmp >>= 1;
			}
			out[index-1] = tmp;
		}

		return std::make_pair(out, offset);
	}

	bool bit_from_stream(size_t n, const std::vector<char>& bit_stream){
		return BIT(n, bit_stream.data());
	}

	std::vector<char> represent(char c, const std::vector<std::pair<char, size_t>>& sorted_pairs){
		std::vector<char> out;
		size_t index = std::find_if(sorted_pairs.cbegin(), sorted_pairs.cend(),
				[c](const std::pair<char, size_t>& p){
					return c == p.first;
				}) - sorted_pairs.cbegin();
		// TODO
		return out;
	}

	std::pair<compressed_data_t, const Node*> compress(const std::string& data){
		using pair_t = std::pair<char, size_t>;
		using table_t = std::map<char, std::vector<char>>;

		compressed_data_t out;
		std::vector<pair_t> vp;
		std::map<char, size_t> mp;

		printf("Initializing...\n");
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
		printf("Obtaining Huffman Tree...\n");
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
			}
		}

		// Obtaining the lookup table
		printf("Obtaining lookup table...\n");
		std::map<char, std::vector<bool>> table;

		for(auto node: nodes_copy){
			std::vector<bool> bits;
			while(node->top){
				bits.push_back(node->top->right->c == node->c);
				node = node->top;
			}
			table[node->c] = std::vector<bool>(bits.crbegin(), bits.crend());
		}

		// Compressing the data
		printf("Compressing the data\n");
		std::vector<bool> bits;
		for(const auto& c: data){
			auto c_bits = table[c];
			for(const auto& bit: c_bits)
				bits.push_back(bit);
		}
		printf("Finializing...\n");
		out = convert_to_bit_stream(bits);
		
		return std::make_pair(out, nodes.front());
	}

	std::string decompress(const compressed_data_t& data, const Node* head){
		std::string out;
		std::vector<bool> bits;
		char c = 0;
		const Node* node = head;

		for(size_t i=0; i<data.first.size()-data.second; ++i){
			size_t index = data.first.size()-1-i;
			bool bit = bit_from_stream(index, data.first);

			if(!node->left && !node->right){
				node = head;
				bits.clear();
				// out += reverse_char(c);
			}
			else if(node->left && !bit){
			}
			else if(node->right && bit){
			}
		}
		
		return out;
	}
}

