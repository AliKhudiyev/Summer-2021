// #include "lint.h"
#include "lint2.h"

using namespace std;

const char* words[10] = {
	"title",
	"car",
	"air",
	"plain",
	"apple",
	"dynamite",
	"game",
	"computer",
	"learn",
	"play"
};

int main(int argc, const char** argv){
	if(argc == 1)
		return -1;

	auto lint = LINT2();
	// lint.train("~/datasets/AGI/enwik9", 5);
	lint.train("/Users/alikhudiyev/datasets/AGI/enwik9", 2);
	printf("Trained: succ\n");

	string word(argv[1]);
	string input;
	size_t extend = 0;
	if(argc > 2)
		extend = std::stoul(string(argv[2]));

	printf("%c\n", lint.predict_char(word));
	
	/*
	for(int i=0; i<word.size(); ++i){
		input += word[i];
		char output = lint.predict_char(input);
		printf("%s-%c\n", input.c_str(), output);
	}
	printf("\n");
	input = word;
	for(int i=0; i<extend; ++i){
		char output = lint.predict_char(input);
		printf("%s-%c\n", input.c_str(), output);
		input += output;
	}
	*/
	return 0;

}
