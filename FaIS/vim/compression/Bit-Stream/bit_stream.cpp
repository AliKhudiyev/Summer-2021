#include "bit_stream.h"

#include <cstdio>


struct BitChunk{
	char chunks[4];
};

struct A{};

struct B: A{};
struct C: A{};

int main(){
	printf("BitStream v0.1 testing...\n\n");

	BitStream bs;
	printf("sizeof BitChunk: %zu\n", sizeof(BitChunk));
	bs.push<BitChunk>({ 'A', 'B', 'C', 'D' }, 8*sizeof(BitChunk), 0, bs.end());
	printf("pushing E...\n");
	bs.push<char>('E', 8, 0, bs.end());
	printf("pushing 0x1...\n");
	bs.push<char>(0x01, 8, 0, bs.rbegin());

	int i=0;
	char c;
	printf("reading 1\n");
	for(auto it=bs.cbegin(); it!=bs.cend(); ++it){
		printf("%d ", *it); ++i;
		if(i % 8 == 0) printf("| ");
	}	printf("\n%d\n", i);
	
	printf("reading 2\n");
	for(auto it=bs.cbegin(); it<bs.cend(); it+=8){
		c = bs.get<char>(it, 8);
		printf("%d(%c) ", c, c);
	}	printf("\n");

	printf("%d, %d, %d, %d, %d\n", 
			bs.begin() == bs.cbegin(), bs.begin() == bs.rend(), bs.begin() == bs.crend()-1, 
			bs.cbegin() == bs.begin(), bs.crbegin()+1 == bs.cend()-2);
	// printf("%d, %d, %d, %d, %d\n",
	// 		bs.cbegin() < bs.end(), bs.rend() < bs.cend(), bs.crbegin() > bs.end(), 2, 2);

	// std::vector<int> iv;
	// const std::vector<int>::const_iterator it = iv.cbegin();
	// iv.begin() == iv.crbegin();

	return 0;
}
