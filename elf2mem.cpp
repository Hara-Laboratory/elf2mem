/**
 * @file
 * @brief Load program from given ELF executable file.
 * @author SAKAMOTO Noriaki <noriakis@cad.ce.titech.ac.jp>
 * @date 2014-08-05
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <libelf.h>
#include <elf.h>
#include <libelf.h>
#include <gelf.h>
#include <math.h>
#define forcenonnull(ptr,...) errorif((ptr)==NULL,__VA_ARGS__)
#define errorif(...) errorif2(__VA_ARGS__,0)
#define errorif2(test,fmt,...) do{if(test){fprintf(stderr, "%s:%s:%d: " fmt "\n",__FILE__, __func__, __LINE__, __VA_ARGS__);exit(EXIT_FAILURE);}}while(0)
#include<cstddef>
#include <ostream>
#include <sstream>
#include "memchunk.h"

#define ALOCLIMIT		0x0d000000		/* default size is 218MB   */
#define MINADDR			0x00001000		/* minimum address is 4KB  */
#define MEMSIZE			0x0e000000		/* default size is 234MB   */
#define HDRADDR		0x00001000

using namespace memory;

#define ELEMENT_PER_LINE 16
#if 0
void print_mem_elem(std::vector<std::ostream> &outs, size_t pos, int v) {
	auto outn = pos % outs.size();
	auto nelem = pos / outs.size();
	const char *header = (nelem % ELEMENT_PER_LINE == 0) ? "\t" : "";
	const char *footer = (nelem % ELEMENT_PER_LINE == (ELEMENT_PER_LINE - 1)) ? ",\n" : ", ";
	outs[outn] << header << std::hex << std::showbase << v << footer;
}

void print_mem_header(const char *name, std::vector<std::ostream> &outs, size_t pos) {
	if (outs.size() > 1) {
		outs[0] << "size_t " << name << "_address = " << std::hex << std::showbase << pos << std::endl;
	}
	for (int i = 0; i < outs.size(); ++i) {
		outs[i] << "char " << name << std::dec << i << " = {" << std::endl;
	}
}

void print_mem_footer(const char *name, std::vector<std::ostream> &outs, size_t pos) {
	for (int i = 0; i < outs.size(); ++i) {
		outs[i] << "};" << std::endl;
	}
}

void print_mem(const char *name, std::vector<std::ostream &> &outs, size_t pos, Memory mem) {
	print_mem_header(name, outs, pos);
	while (!mem.empty()) {
		auto t = mem.popChunk();
		size_t addr = t.first;
		auto ch = t.second.getContaint();
		for (; pos < addr; ++pos) {
			print_mem_elem(outs, pos, 0);
		}
		for (; pos < addr + ch.size(); ++pos) {
			print_mem_elem(outs, pos, ch[addr - pos]);
		}
	}
	print_mem_footer(name, outs, pos);
}

#else
void print_mem_elem(std::vector<FILE *>outs, size_t pos, int v) {
	auto fp = outs[pos % outs.size()];
	auto nelem = pos / outs.size();
	const char *header = (nelem % ELEMENT_PER_LINE == 0) ? "\t" : "";
	const char *footer = (nelem % ELEMENT_PER_LINE == (ELEMENT_PER_LINE - 1)) ? ",\n" : ", ";
	fprintf(fp, "%s0x%02hhx%s", header, (char)v, footer);
}

void print_mem_header(const char *name, std::vector<FILE *>outs, size_t pos) {
	if (outs.size() > 1) {
		fprintf(outs[0], "size_t %s_address_begin = 0x%zx;\n", name, pos);
	}
	for (int i = 0; i < outs.size(); ++i) {
		fprintf(outs[i], "char %s%d[] = {\n", name, i);
	}
}

void print_mem_footer(const char *name, std::vector<FILE *>outs, size_t pos) {
	for (int i = 0; i < outs.size(); ++i) {
		fprintf(outs[i], "};\n");
	}
	fprintf(outs[0], "size_t %s_address_end = 0x%zx;\n", name, pos);
}

void print_mem(const char *name, std::vector<FILE *>outs, size_t pos, Memory mem) {
	print_mem_header(name, outs, pos);
	while (!mem.empty()) {
		auto t = mem.popChunk();
		size_t addr = t.first;
		auto ch = t.second.getContaint();
		for (; pos < addr; ++pos) {
			print_mem_elem(outs, pos, 0);
		}
		for (; pos < addr + ch.size(); ++pos) {
			print_mem_elem(outs, pos, ch[pos - addr]);
		}
	}
	print_mem_footer(name, outs, pos);
}
#endif

/**
 * Read data of given section from ELF file.
 *
 * @param elf file
 * @param index of section to read
 * @return pointer to malloced data
 */
static char *read_section(Elf *elf, size_t index) {
	Elf_Scn *scn = elf_getscn(elf, index);
	forcenonnull(scn,"elf_getscn() failed\n");

	Elf32_Shdr *shdr = elf32_getshdr(scn);
	forcenonnull(shdr,"elf32_getshdr() failed\n");

	char *dst = (char *)malloc(shdr->sh_size);
	forcenonnull(dst,"malloc() failed\n");

	Elf_Data *data = NULL;
	size_t n = 0;

	while (n < shdr->sh_size &&
			(data = elf_getdata(scn, data)) != NULL) {
		memcpy(dst + n, data->d_buf, data->d_size);
		n += data->d_size;
	}

	return dst;
}

/**
 * Read entry point from ELF file.
 *
 * @param elf file
 * @return entry point
 */
static size_t read_entry(Elf *elf) {
	Elf32_Ehdr *ehdr = elf32_getehdr(elf);
	forcenonnull(elf, "elf32_getehdr() failed");

	return ehdr->e_entry;
}

/* READ_ELF() */
unsigned int read_elf(Memory &mem, const char *file) {
	/* FILEOPEN */
	FILE *fp;
	int i;

	Elf_Data *data;
	GElf_Sym dst;

	fp = fopen(file, "r");
	if (fp == NULL){
		printf("Can't open elf_file\n");
		exit(1);
	}

	int fd = fileno(fp);

	elf_version(EV_CURRENT);
	Elf *elf = elf_begin(fd, ELF_C_READ, NULL);
	forcenonnull(elf, "elf_begin() failed");

	/* READ ELF_HEADER */
	size_t shnum;
	if (elf_getshdrnum (elf, &shnum) != 0) {
		fprintf(stderr, "elf_getshdrnum() failed");
		exit(EXIT_FAILURE);
	}

	Elf32_Shdr	*Shead = (Elf32_Shdr *)malloc(shnum * sizeof(Elf32_Shdr));
	size_t symtabndx;
	for (size_t i = 0;  i < shnum; ++i) {
		Elf_Scn *scn = elf_getscn(elf, i);
		Elf32_Shdr *shdr = elf32_getshdr(scn);
		Shead[i] = *shdr;
		if (shdr->sh_type == SHT_SYMTAB)
			symtabndx = i;
	}

	size_t shstrndx;
	if (elf_getshdrstrndx (elf , &shstrndx) != 0) {
		fprintf(stderr, " elf_getshdrstrndx () failed. ");
		exit(EXIT_FAILURE);
	}

	unsigned int ptrheap = 0;
	bool found_text = false, found_data = false, found_bss = false, found_rodata = false;
	size_t textndx, datandx, bssndx, strtabndx, rodatandx;
	for (size_t i = 0; i < shnum; ++i) {
		char *name;
		if ((name = elf_strptr(elf, shstrndx, Shead[i].sh_name)) == NULL) {
			fprintf(stderr, "elf_strptr() failed.\n");
			exit(EXIT_FAILURE);
		}
		//printf("section %s: addr=%x offset=%x link=%x\n", name, Shead[i].sh_addr, Shead[i].sh_offset, Shead[i].sh_link);

		if		(!strcmp(name, ".text")) {
			found_text = true;
			textndx = i;
			ptrheap = std::max(ptrheap, Shead[i].sh_addr + Shead[i].sh_size);
		} 
		else if (!strcmp(name, ".data")) {
			found_data = true;
			datandx = i;
			ptrheap = std::max(ptrheap, Shead[i].sh_addr + Shead[i].sh_size);
		} 
		else if (!strcmp(name, ".bss")) {
			found_bss = true;
			bssndx = i;
			ptrheap = std::max(ptrheap, Shead[i].sh_addr + Shead[i].sh_size);
			//	  else if (!strcmp(name, ".symtab"))
			//		symtabndx = i;
		} 
		else if (!strcmp(name, ".rodata")) {
			found_rodata = true;
			rodatandx = i;
			ptrheap = std::max(ptrheap, Shead[i].sh_addr + Shead[i].sh_size);
		}
		else if (!strcmp(name, ".strtab")) {
			strtabndx = i;
		}
	}

	errorif(!found_text, "not found .text section");

	/* READ PROGRAM_HEADER */
	size_t phdrnum;
	if (elf_getphdrnum(elf, &phdrnum) != 0) {
		fprintf(stderr, "elf_getphdrnum failed.\n");
		exit(EXIT_FAILURE);
	}

	if (phdrnum == 0) {
		fprintf(stderr, "the file has no program headers.\n");
		exit(EXIT_FAILURE);
	}

	GElf_Phdr *Phead = (GElf_Phdr *)malloc(phdrnum * sizeof(GElf_Phdr));
	forcenonnull(Phead, "malloc of Phead failed.");

	for (size_t i = 0; i < phdrnum; i++) {
		if (gelf_getphdr(elf, i, &Phead[i]) != &Phead[i]) {
			fprintf(stderr, "gelf_getphdr() failed");
			exit(EXIT_FAILURE);
		}
		//printf(YELLOW"program %ld size=%8.8lx, offset=%8.8lx\n"RESET, i, Phead[i].p_filesz, Phead[i].p_offset);
	}


	/* READ (text, data) */
	if (found_text) {
		char *Text_p = read_section(elf, textndx);
		std::vector<char> v( Text_p, Text_p + Shead[textndx].sh_size);
		Chunk ch(v);
		mem.addChunk(Shead[textndx].sh_addr, ch);
		free(Text_p);
	}

	if (found_rodata) {
		char *Rodata_p = read_section(elf, rodatandx);
		std::vector<char> v( Rodata_p, Rodata_p + Shead[rodatandx].sh_size);
		Chunk ch(v);
		mem.addChunk(Shead[rodatandx].sh_addr, ch);
		free(Rodata_p);
	}

	if (found_data) {
		char *Data_p = read_section(elf, datandx);
		std::vector<char> v( Data_p, Data_p + Shead[datandx].sh_size);
		Chunk ch(v);
		mem.addChunk(Shead[datandx].sh_addr, ch);
		free(Data_p);
	}

	mem.entry(Phead[0].p_vaddr);
	Elf32_Addr e_entry = read_entry(elf);
	Elf32_Addr a_entry = Phead[0].p_vaddr;

	printf("e_entry=%8.8x\n" , e_entry);

	free(Shead);
	free(Phead);
	// free(bss_p);
	elf_end(elf);
	fclose(fp);
	return a_entry;
}

int main (int argc, char **argv) {
	if (argc != 4) {
		printf("USAGE: %s <input_file> <output_file> <name>", argv[0]);
	}

	char *input_file = argv[1];
	char *output_file = argv[2];
	char *output_name = argv[3];

	Memory mem;
	Elf32_Addr addr = read_elf(mem, input_file);

	std::vector<FILE *> outs;
	// std::vector<std::ostringstream> outs;
	for (int i = 0; i < 4; ++i) {
		std::ostringstream ostr;
		ostr << output_file << i << ".h";
		auto fp = fopen(ostr.str().c_str(), "w");
		outs.push_back(fp);
		// outs.push_back(ostr);
	}
	print_mem(output_name, outs, addr, mem);
	return 0;
}

// vim: ts=8 sw=2 :
