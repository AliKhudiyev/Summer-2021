# BitStream (C++ Library)

BitStream is a single header C++ library written for the management of stream(s) of bits.

```c++
#include "bit_stream.h"
#include <cstdio>

int main(){
	char c = 'A';	// 0x41

	BitStream bs;
	bs.push(&c, 8, 0);
	bs.push<uint8_t>(174, 3, 2, bs.begin());

	char c_ = bs.get<char, 8>(bs.cbegin());	// 0x41
	return 0;
}
```

*Note:* The BitStream library is to be used in my other compression-related projects.
**IN DEVELOPMENT!**

