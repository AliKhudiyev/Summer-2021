#include "bit_stream.h"

#include <cstdio>
#include <iostream>

struct BitChunk{
	char c[4];
};

int main(){
	BitChunk bc { 0x41, 0x42, 0x43, 0x44 };
        
	BitStream bs;
	bs.push<BitChunk>(bc, bs.begin());
	auto chunk = bs.get<BitChunk>(bs.begin());
	
	printf("%c %c %c %c\n", 
		chunk.c[0], chunk.c[1], chunk.c[2], chunk.c[3]);
	
	return 0;
}
