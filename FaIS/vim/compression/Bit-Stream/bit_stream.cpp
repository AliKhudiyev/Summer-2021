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

	// bs.rotate(3*bs.size()+8, bs.rbegin(), bs.rend());
	// bs.rotate(1, bs.rbegin()+1, bs.rbegin()+28);
	// bs.shift(1, bs.rbegin()+1, bs.rend()-1);
	// bs.rotate(1, bs.rbegin()+1, bs.rend()-1);
	// bs.memmov(3, bs.rbegin()+13, bs.crend()-8);
	bs.insert<char>('z', bs.begin()+16);
	// bs.memcpy(8, bs.begin()+16, bs.crend()-24);
	bs.memflp(8, bs.begin()+16);
	bs.memflp(8, bs.rbegin()+16);
	printf("reading 2\n");
	bs.print();

	return 0;
}
