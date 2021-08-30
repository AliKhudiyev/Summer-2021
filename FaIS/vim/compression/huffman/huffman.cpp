#include "huffman.h"

using namespace std;
using namespace huffman;

int main(){
	string text = "abaabc";
	auto encoded = compress(text);
	string decoded = decompress(encoded.first, encoded.second);

	print("%s\n\n%s\n", text.c_str(), decoded.c_str());
/*
	auto bit_stream = convert_to_bit_stream({0, 1, 0, 0, 0, 0, 0, 1});
	printf("%zu : %c(%d)\n", bit_stream.size(), bit_stream.front(), bit_stream.front());
*/
	return 0;
}
