# BitStream (C++ Library)

BitStream is a single header C++ library written for the management of stream(s) of bits.

```c++
#include "bit_stream.h"
#include <cstdio>

struct BitChunk{
	char bytes[4];
}

int main(){
	BitStream bs;

	char c = 'A';	// 0x41
	bs.push(&c, 8, 0, bs.end());

	// pushing custom types
	bs.push<uint8_t>(174, 3, 2, bs.end());
	bs.push<BitChunk>({ 'B', 'C', 'D', 'E' }, 8*sizeof(BitChunk), 0, bs.end())

	// reading a byte
	char c_ = bs.get<char>(bs.cbegin(), 8);	// 0x41

	// printing the bits in the stream
	for(auto it=bs.cbegin(); it!=bs.cend(); ++it){
		printf("%d ", *it);
	}	printf("\n");

	// printing the characters in the stream
	for(auto it=bs.cbegin(); it!=bs.cend(); it+=8){
		printf("%c ", bs.get<char>(it, 8));
	}	printf("\n");

	return 0;
}
```

*Note:* The BitStream library is to be used in my other compression-related projects.
**IN DEVELOPMENT!**

