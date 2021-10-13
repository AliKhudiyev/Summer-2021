#include "alc.h"
#include <getopt.h>
#include <cstdlib>

#define SHOW_USAGE_OF(cmd, help)											\
	command = cmd; tmp_spaces = spaces - command.size();					\
	format[3] = '0' + tmp_spaces/10; format[4] = '0' + tmp_spaces%10;		\
	printf(format.c_str(), command.c_str(), " ", help);


using namespace alc;
using namespace std;

int main(int argc, char* const* argv){
	std::string system_in, system_out;
	std::string options_in, options_out;
	std::string data_in, data_out;
	std::string stats_out;
	int mode = 0, verbose = 0, log = 0;
	float speed = 5.f;

	while(1){
		static option long_options[] = {
			{"help",		no_argument, 		0,			'h'},
			{"verbose",		no_argument, 		0,			'v'},
			{"system_in",	required_argument,	0,			'i'},
			{"options_in",	required_argument, 	0, 			'p'},
			{"data_in",		required_argument, 	0, 			'I'},
			{"system_out",	required_argument, 	0, 			'o'},
			{"options_out",	required_argument, 	0, 			'P'},
			{"data_in",		required_argument, 	0, 			'O'},
			{"stats_out",	required_argument, 	0, 			'T'},
			{"mode",		required_argument, 	0, 			'm'},
			{"speed",		required_argument, 	0, 			's'},
			{"log",			required_argument, 	0, 			'l'},
		};

		int option_index = 0;
		char c = getopt_long_only(argc, argv, "hvi:p:I:o:P:O:T:m:s:l", long_options, &option_index);

		if(c == -1)
			break;

		switch(c){
			case 'h':
			{
				std::string command = "--help -h";
				std::string format = "%s%00s%s\n";
				const int spaces = 40;
				int tmp_spaces = 0;

				printf("Usage: ./alc [options]\n");
				SHOW_USAGE_OF("--help, -h", "show this and exit");
				SHOW_USAGE_OF("--verbose, -v", "run-time info");
				SHOW_USAGE_OF("--system_in, -i [path]", "system path to be loaded");
				SHOW_USAGE_OF("--options_in, -p [path]", "options path to be loaded");
				SHOW_USAGE_OF("--data_in, -I [path]", "dataset path to be loaded");
				SHOW_USAGE_OF("--system_out, -o [path]", "to store the system");
				SHOW_USAGE_OF("--options_out, -P [path]", "to store the options");
				SHOW_USAGE_OF("--data_out, -O [path]", "to store the dataset");
				SHOW_USAGE_OF("--stats_out, -T [path]", "to store the statistics");
				SHOW_USAGE_OF("--mode, -m [0/1/2]", "[0]: fit & predict, 1: fit, 2: predict");
				SHOW_USAGE_OF("--speed, -s [value]", "[5.0] run-time speed, range [1, 10]");
			}	exit(0);
			case 'v':
				verbose = true;
				break;
			case 'i':
				system_in = std::string(optarg);
				break;
			case 'p':
				options_in = std::string(optarg);
				break;
			case 'I':
				data_in = std::string(optarg);
				break;
			case 'o':
				system_out = std::string(optarg);
				break;
			case 'P':
				options_out = std::string(optarg);
				break;
			case 'O':
				data_out = std::string(optarg);
				break;
			case 'T':
				stats_out = std::string(optarg);
			case 'm':
				mode = atoi(optarg);
				if(mode < 0 || mode > 2)
					mode = 0;
				break;
			case 's':
				speed = atof(optarg);
				if(speed < 1) speed = 1.f;
				else if(speed > 10) speed = 10.f;
				break;
			case 'l':
				log = atoi(optarg);
				break;
			default: exit(1);
		}
	}

	printf("in.sys: %s | out.sys: %s\n", system_in.c_str(), system_out.c_str());
	printf("in.opt: %s | out.opt: %s\n", options_in.c_str(), options_out.c_str());
	printf("in.csv: %s | out.csv: %s\n", data_in.c_str(), data_out.c_str());
	printf("out.sta: %s\n", stats_out.c_str());
	printf("mode: %d | speed: %.1f | verbose: %d | log: %d\n", mode, speed, verbose, log);

	IO io;
	if(io.load(data_in.c_str()) != LOAD_OK){
		printf("Invalid dataset path: [%s]\n", data_in.c_str());
		exit(1);
	}

	Options options;
	Policy policy;

	System* system;
	if(system_in.empty() || options_in.empty())
		system = new System(system_in.c_str(), options_in.c_str(), options);
	else
		system = new System(io.input_count(), io.output_count(), policy, options);

	if(mode == 1){ // Fit
		system->fit(io);
	}
	else if(mode == 2){ // Predict
		system->predict(io);
	}
	else{ // Fit & Predict
		raw_output_t out_pred;
		auto stats = system->get_stats();
		for(size_t i=0; i<io.size(); ++i){
			out_pred = system->fit_predict(io.input_at(i), io.output_at(i));
		}
	}

	delete system;
	return 0;
}
