#include "bit_stream.h"

#include <cstdio>
#include <iostream>

struct BitChunk{
	char chunks[4];
};

struct A{};
struct B: A{};
struct C: B{};

void test(A obj){
	printf("worked!\n");
}

int main(){
	printf("BitStream v0.1 testing...\n\n");


	BitStream bs;
	printf("sizeof BitChunk: %zu\n", sizeof(BitChunk));
	bs.push<BitChunk>({ 'A', 'B', 'C', 'D' }, bs.end());
	printf("pushing E...\n");
	bs.push<char>('E', bs.end());
	int tmp = 0x04030201;
	uint8_t* ptr = reinterpret_cast<uint8_t*>(&tmp);
	printf("bytes: %.2x %.2x %.2x %.2x\n", ptr[0], ptr[1], ptr[2], ptr[3]);
	printf("pushing %.8x...\n", tmp);
	bs.push<int>(tmp, bs.end());
	// bs.set<int>(tmp, bs.begin());

	printf("reading 1\n");
	bs.print(std::cout, true);
	printf("bit count: %zu\n", bs.size());
	// bs.insert(true, bs.rbegin());

	// bs.rotate(1, bs.begin(), bs.end());
	// bs.rotate(1, bs.rbegin()+1, bs.rbegin()+28);
	// bs.shift(1, bs.rbegin()+1, bs.rend()-1);
	// bs.rotate(1, bs.rbegin()+1, bs.rend()-1);
	
	// bs.memmov(3, bs.begin()+13, bs.crend()-8);
	// bs.memmov(3, bs.begin()+13, bs.cend()-10);
	// bs.memmov(3, bs.rbegin()+13, bs.crend()-8);
	// bs.memmov(3, bs.rbegin()+13, bs.cend()-10);
	
	// bs.insert<int>(0x05060708, bs.begin()+16, 16);
	// bs.insert<int>(0x05060708, bs.rend()-16, 16);
	
	// bs.memcpy(8, bs.begin()+16, bs.crend()-24);
	// bs.memmov(8, bs.rend()-8, bs.cbegin());
	// bs.memflp(8, bs.begin()+16);
	// bs.memflp(7, bs.begin()+2);
	// bs.memflp(19, bs.rbegin()+6);
	
	// bs.set<char>(65, bs.rend()-8);
	// bs.set<char>(65, bs.begin());
	// printf("get results: %u and %u\n", bs.get<uint8_t>(bs.cbegin()), bs.get<uint8_t>(bs.crend()-8));
	printf("reading 2\n");
	bs.print();

	return 0;
}
