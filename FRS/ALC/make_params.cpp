#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include "utils.h"

using namespace alc::utils;
using namespace alc::parameters;

int main(int argc, char** argv){
	if(argc < 2){
		fprintf(stderr, "Usage: ./make_params [file_path]\n");
		return 1;
	}

	const char* const file_path = argv[1];
	Options options;
	Policy policy;
	std::map<std::string, std::string> mp;

	std::string command = "help", key, value;
	printf("Initialized parameters...\n");

	while(1){
		if(command.substr(0, 2) == "ls"){ // ls [options/policy]
			if(command.size() == 2){
				print_params(options);
				print_params(policy);
			}
			else if(command.size() == 10 && command.substr(3) == "options"){
				print_params(options);
			}
			else if(command.size() == 9 && command.substr(3) == "policy"){
				print_params(policy);
			}
			else{
				fprintf(stderr, "Usage: `ls` or `ls options` or `ls policy`\n");
			}
		}

		else if(command.substr(0, 5) == "reset"){ // reset [options/policy]
			if(command.size() == 5){
				options = Options();
				policy = Policy();
			}
			else if(command.size() == 13 && command.substr(6) == "options"){
				options = Options();
			}
			else if(command.size() == 12 && command.substr(6) == "policy"){
				policy = Policy();
			}
			else{
				fprintf(stderr, "Usage: `reset` or `reset options` or `reset policy`\n");
			}
		}

		else if(command.substr(0, 3) == "set"){ // set [key] [value]
			size_t index = command.find(' ', 4);
			if(index != -1){
				key = command.substr(4, index-4);
				printf("%s\n", key.c_str());
				if(is_valid_param(key.c_str(), options, policy)){
					value = command.substr(index+1);
					mp[key] = value;
					set_param(key.c_str(), value.c_str(), options, policy);
					printf("< set %s = %s\n", key.c_str(), value.c_str());
				}
				else{
					fprintf(stderr, "Key not found!\n");
				}
			}
			else{
				fprintf(stderr, "Usage: `set [key] [value]`\n");
			}
		}

		else if(command.substr(0, 4) == "info"){ // info [key]
			if(command.size() > 5){
				key = command.substr(5);
				auto info = get_info(key.c_str(), options, policy);
				if(info.size()){
					printf("%s\n", info.c_str());
				}
				else{
					fprintf(stderr, "Key not found!\n");
				}
			}
			else{
				fprintf(stderr, "Usage: `info [key]`\n");
			}
		}

		else if(command.substr(0, 4) == "load"){ // load [file]
			if(command.size() > 5){
				auto file_path = command.substr(5);
				if(load_params(file_path.c_str(), options, policy) == LOAD_OK)
					printf("< loaded\n");
				else
					fprintf(stderr, "File could not be loaded!\n");
			}
			else{
				fprintf(stderr, "Usage: `load [file]`\n");
			}
		}

		else if(command.substr(0, 4) == "help"){
			printf("help - to show this\n");
			printf("ls [options/policy] - to display parameters\n");
			printf("reset [options/policy] - to reset parameters\n");
			printf("set [key] [value] - to set value of the parameter(key)\n");
			printf("info [key] - to display parameter info\n");
			printf("load [file] - to load parameters from a file\n");
			printf("save or s - to save\n");
			printf("quit or q - to quit\n");
			printf("sq - to save and quit\n");
		}

		else if(command == "s" || command == "save"){
			if(!save_params(file_path, options, policy))
				printf("< saved\n");
		}

		else if(command == "q" || command == "quit"){
			break;
		}

		else if(command == "sq"){
			if(!save_params(file_path, options, policy))
				printf("< saved\n");
			break;
		}

		else{
			fprintf(stderr, "Unknown command, type `help`.\n");
		}

		printf("> ");
		std::getline(std::cin, command);
	}
	return 0;
}
