#include "huffman.h"

using namespace std;
using namespace huffman;

int main(int argc, const char** argv){
	string text = "hello, my name is world and this is an example of huffman compression.";
	if(argc == 2) text = string(argv[1]);
	printf("Original:   %s\n", text.c_str());
	auto encoded = compress(text);
	string decoded = decompress(encoded.first, encoded.second);

	printf("Compressed: %s\n", decoded.c_str());
	printf("Compression Rate: %.3f\n", (float)(text.size()-encoded.first.first.size())/text.size());
	return 0;
}
