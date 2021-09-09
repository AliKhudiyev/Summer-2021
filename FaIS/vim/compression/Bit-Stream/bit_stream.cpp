#include "bit_stream.h"

#include <cstdio>
#include <iostream>

struct BitChunk{
	char chunks[4];
};

int main(){
	printf("BitStream v0.1 testing...\n\n");

	BitStream bs(8, 1);
	bs.print();

	return 0;
}
