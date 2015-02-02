/**
 * @file
 * @brief Load program from given ELF executable file.
 * @author SAKAMOTO Noriaki <noriakis@cad.ce.titech.ac.jp>
 * @date 2015-01-30
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <libelf.h>
#include <math.h>
#include <getopt.h>

#include<cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include "memchunk.h"
#include "printerC.h"
#include "readerElf.h"

#define MEMORY_SIZE 0x0e000000

using namespace memory;

void printusage(const char *name, std::ostream &ost) {
	ost << "USAGE: " << name << " [options] [-o <output_file>] <input_file> " << std::endl;
	ost << std::endl;
	ost << "OPTIONS:" << std::endl;
	ost << "\t-b <addr>\tStart address." << std::endl;
	ost << "\t-B\tOutput from the first section. (default)" << std::endl;
	ost << "\t-e <addr>\tEnd address." << std::endl;
	ost << "\t-E\tOutput until end of the sections. (default)" << std::endl;
	ost << "\t-t <type>\tOutput type." << std::endl;
	ost << "\t\t\tAvailable choices: 'c-array' (default)" << std::endl;
	ost << "\t-o <filename>\tOutput file name (scheme). (default: basename of input file + \".h\")" << std::endl;
	ost << "\t-s <num>\tSplit number. (default: 1)" << std::endl;
	ost << "\t-w <num>\tOutput bitwidth." << std::endl;
	ost << "\t\t\tAvailable choices: 8 (default), 16, 32, 64" << std::endl;
	ost << "\t--byte-order <string>\tOutput byte order." << std::endl;
	ost << "\t\t\tAvailable choices: little-endian, big-endian (default)" << std::endl;
	ost << std::endl;
	ost << "for output type 'c-array':" << std::endl;
	ost << "\t-n <name>\tIdentifier name. (default: basename of input file)" << std::endl;
}

int main (int argc, char **argv) {
	const char *input_file = NULL;
	std::string *output_file = NULL;
	const char *output_name = NULL;
	bool begin_address_set = false;
	size_t begin_address = 0;
	bool end_address_set = false;
	size_t end_address = 0;
	int split = 1;
	int width = 8;

	static struct option long_options[] = {
		{"start-address",  	required_argument, 	0,  'b' },
		{"start-minimal",  	no_argument, 		0,  'B' },
		{"end-address",  	required_argument, 	0,  'e' },
		{"end-minimal",  	no_argument, 		0,  'E' },
		{"type",  			required_argument, 	0,  't' },
		{"output",  		required_argument, 	0,  'o' },
		{"split",  			required_argument, 	0,  's' },
		{"bitwidth",		required_argument, 	0,  'w' },
		{"byte-order",  		required_argument, 	0,  0 },
		{"identifier",		required_argument, 	0,  'n' },
		{0,         0,                 0,  0 }
	};
	int opt;
	while (1) {
		int option_index = 0;
		opt = getopt_long(argc, argv, "t:o:b:e:En:w:s:", long_options, &option_index);
		if (opt == -1)
			break;

		switch (opt) {
			case 0:
				if (!strcmp(long_options[option_index].name, "byte-order")) {
					if (!strcmp(optarg, "little-endian")) {
						std::cerr << "little endian is not supported yet!" << std::endl;
						printusage(argv[0], std::cerr);
						exit(EXIT_FAILURE);
					} else if (!strcmp(optarg, "big-endian")) {
					} else {
						std::cerr << "unrecognizable byte-order: " << optarg << std::endl;
						printusage(argv[0], std::cerr);
						exit(EXIT_FAILURE);
					}
				}
				break;
			case 't':
				if (!!strcmp(optarg, "c-array")) {
					std::cerr << "unrecognizable output type: " << optarg << std::endl;
					printusage(argv[0], std::cerr);
					exit(EXIT_FAILURE);
				}
				break;
			case 'o':
				output_file = new std::string(optarg);
				break;
			case 'b':
			{
				begin_address = std::stoull(optarg, NULL, 0);
				begin_address_set = true;
				break;
			}
			case 'B':
				begin_address_set = false;
				break;
			case 'e':
			{
				end_address = std::stoull(optarg, NULL, 0);
				end_address_set = true;
				break;
			}
			case 'E':
				end_address_set = false;
				break;
			case 'n':
				output_name = optarg;
				break;
			case 'w':
			{
				width = std::stoi(optarg);
				break;
			}
			case 's':
			{
				std::istringstream istr(optarg);
				istr >> split;
				if (split <= 0) {
					std::cerr << "split number must be greater than zero." << std::endl;
				}
				break;
			}
			default:
				std::cerr << "unknown option." << std::endl;
				exit(EXIT_FAILURE);
		}
	}

	if (optind == argc - 1) {
		input_file = argv[optind];
	} else if (optind == argc) {
		std::cerr << "input file is not given" << std::endl;
		printusage(argv[0], std::cerr);
		exit(EXIT_FAILURE);
	} else {
		std::cerr << "too many arguments" << std::endl;
		printusage(argv[0], std::cerr);
		exit(EXIT_FAILURE);
	}

	if (output_file == NULL) {
		std::ostringstream ostr;
		ostr << basename(input_file) << ".h";
		output_file = new std::string(ostr.str());
	}
	std::cout << "output: " << *output_file << std::endl;

	if (output_name == NULL) {
		output_name = basename(input_file);
	}
	std::cout << "identifier: " << output_name << std::endl;

	FILE *fp = fopen(input_file, "r");
	if (fp == NULL){
		printf("Can't open elf_file\n");
		exit(1);
	}

	Memory mem;
	read_elf(mem, fp);

	/*
	// std::vector<FILE *> outs;
	std::vector<FILE *> outs;
	// std::vector<std::ostringstream> outs;
	for (int i = 0; i < 4; ++i) {
		std::ostringstream ostr;
		ostr << output_file << i << ".h";
		auto fp = fopen(ostr.str().c_str(), "w");
		outs.push_back(fp);
		// outs.push_back(ostr);
	}
	*/

	std::ofstream ofs(output_file->c_str());
	printerC printerc(output_name, begin_address_set, begin_address, end_address_set, end_address, split, width);
	printerc.print_mem(ofs, mem);

	return 0;
}
