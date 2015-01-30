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
#include <elf.h>
#include <libelf.h>
#include <math.h>

#include<cstddef>
#include <ostream>
#include <fstream>
#include <sstream>
#include "memchunk.h"
#include "printerC.h"
#include "readerElf.h"

#define MEMORY_SIZE 0x0e000000

using namespace memory;

int main (int argc, char **argv) {
	if (argc != 4) {
		printf("USAGE: %s <input_file> <output_file> <name>", argv[0]);
	}

	char *input_file = argv[1];
	char *output_file = argv[2];
	char *output_name = argv[3];

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

	std::vector<std::ostream *> outstreams;
	std::vector<std::ostringstream *> outstrstreams;
	for (int i = 0; i < 4; ++i) {
		auto ostr = new std::ostringstream();
		outstreams.push_back(ostr);
		outstrstreams.push_back(ostr);
		// outs.push_back(ostr);
	}
	print_mem(output_name, outstreams, addr, addr + 0x200, mem);

	std::ofstream ofs(output_file);
	for (auto && ostr : outstrstreams) {
		ofs << ostr->str();
	}
	return 0;
}
