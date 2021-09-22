#include "lzw.h"

using namespace std;
using namespace lzw;

int main(int argc, const char** argv){
	string text = "hello, my name is world and this is an example of lzw compression.";
	auto encoded = compress(text);
	string decoded = decompress(encoded);
	printf("%s\n", decoded.c_str());
	printf("Compression Ratio: %.3f\n", (float)(text.size()-encoded.size())/text.size());
	printf("%zu - %zu\n", encoded.size(), text.size());
	return 0;
}
