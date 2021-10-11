#include "alc.h"
#include <cstdio>
#include <iostream>
#include <cstdlib>

using namespace std;
using namespace alc;
using namespace alc::core;

class Generator: public IO{
	public:
	Generator(){
		m_input_count = 2;
		m_output_count = 1;
		expression = "i_0 ^ i_1";
	}
	~Generator() = default;

	raw_output_t eval(const raw_input_t& input) const{
		raw_output_t output(1);
		output[0] = input[0] ^ input[1];
		return output;
	}

	std::string eval(const std::string& input) const{
		return IO::eval(input);
	}

	void eval(size_t set_count=-1, bool repetitions=true){
		IO::eval(set_count, repetitions);
	}
};

int main(){
	srand(time(0));
	printf("Hello, World!\n");

	IO io;
	// io.generate(4, 2);
	// io.generate(3, 1, 8, 2, 3, 0.5f);
	// System sys = io.system();
	// sys.save("test.sys");
	// auto exprs = sys.to_strings();
	// printf("From main:\n");
	// for(const auto& expr: exprs)
	// 	cout << expr << endl;
	Generator generator;
	// printf("%s -> %s\n", "00", generator.eval("00").c_str());
	// printf("%s -> %s\n", "01", generator.eval("01").c_str());
	// printf("%s -> %s\n", "10", generator.eval("10").c_str());
	// printf("%s -> %s\n", "11", generator.eval("11").c_str());
	generator.eval();
	generator.print();

	System system(2, 1, Policy(), Options());
	system.fit(generator);
	printf("Terminated: succ\n");

	return 0;
}
