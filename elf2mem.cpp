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
#include <unistd.h>

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
	ost << "OPTIONS:" << std::endl;
	ost << "\t-b <addr>\tStart address." << std::endl;
	ost << "\t-B\toutput from the first section." << std::endl;
	ost << "\t-e <addr>\tEnd address." << std::endl;
	ost << "\t-E\tOutput until end of the sections. (default)" << std::endl;
	ost << "\t-t <type>\tOutput type. (one of 'c-array')" << std::endl;
	ost << "\t-o <filename>\tOutput file name (scheme)." << std::endl;
	ost << std::endl;
	ost << "for output type 'c-array':" << std::endl;
	ost << "\t-n <name>\tIdentifier name." << std::endl;
}

int main (int argc, char **argv) {
	if (argc != 4) {
	}

	const char *input_file = NULL;
	const char *output_file = NULL;
	const char *output_name = NULL;
	bool begin_address_set = false;
	size_t begin_address;
	bool end_address_set = false;
	size_t end_address;

	int opt;
	while ((opt = getopt(argc, argv, "t:o:b:e:En:")) != -1) {
		switch (opt) {
			case 't':
				if (!strcmp(optarg, "c-array")) {
					std::cerr << "unrecognizable output type: " << optarg << std::endl;
					printusage(argv[0], std::cerr);
					exit(EXIT_FAILURE);
				}
				break;
			case 'o':
				output_file = optarg;
				break;
			case 'b':
			{
				std::istringstream istr(optarg);
				istr >> begin_address;
				begin_address_set = true;
				break;
			}
			case 'B':
				begin_address_set = false;
				break;
			case 'e':
			{
				std::istringstream istr(optarg);
				istr >> end_address;
				end_address_set = true;
				break;
			}
			case 'E':
				end_address_set = false;
				break;
			case 'n':
				output_name = optarg;
				break;
		}
	}

	if (optind == argc - 1) {
		input_file = argv[optind];
	} else {
		std::cerr << "input file is not given" << std::endl;
		printusage(argv[0], std::cerr);
		exit(EXIT_FAILURE);
	}

	FILE *fp = fopen(input_file, "r");
	if (fp == NULL){
		printf("Can't open elf_file\n");
		exit(1);
	}

	Memory mem;
	Elf32_Addr addr = read_elf(mem, fp);

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

	std::ofstream ofs(output_file);
	print_mem(output_name, ofs, addr, addr + 0x200, mem);

	return 0;
}
