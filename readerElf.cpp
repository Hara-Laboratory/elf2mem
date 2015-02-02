#include <stdlib.h>
#include <string.h>
#include <libelf.h>
#include <gelf.h>
#include "readerElf.h"
#include "memchunk.h"

#define forcenonnull(ptr,...) errorif((ptr)==NULL,__VA_ARGS__)
#define errorif(...) errorif2(__VA_ARGS__,0)
#define errorif2(test,fmt,...) do{if(test){fprintf(stderr, "%s:%s:%d: " fmt "\n",__FILE__, __func__, __LINE__, __VA_ARGS__);exit(EXIT_FAILURE);}}while(0)

using namespace memory;

/**
 * Read data of given section from ELF file.
 *
 * @param elf file
 * @param index of section to read
 * @return pointer to malloced data
 */
static unsigned char *read_section(Elf *elf, size_t index) {
	Elf_Scn *scn = elf_getscn(elf, index);
	forcenonnull(scn,"elf_getscn() failed\n");

	Elf32_Shdr *shdr = elf32_getshdr(scn);
	forcenonnull(shdr,"elf32_getshdr() failed\n");

	unsigned char *dst = (unsigned char *)malloc(shdr->sh_size);
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
 * Read data of given section from ELF file to Memory.
 *
 * @param elf file
 * @param index of section to read
 * @return pointer to malloced data
 */
static void read_section(Memory &mem, Elf *elf, size_t index) {
	unsigned char *buf = read_section(elf, index);

	Elf_Scn *scn = elf_getscn(elf, index);
	forcenonnull(scn,"elf_getscn() failed\n");

	Elf32_Shdr *shdr = elf32_getshdr(scn);
	forcenonnull(shdr,"elf32_getshdr() failed\n");

	std::vector<unsigned char> v(buf, buf + shdr->sh_size);
	Chunk ch(v);
	mem.addChunk(shdr->sh_addr, ch);
	free(buf);
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
unsigned int read_elf(Memory &mem, FILE *fp) {
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

	size_t shstrndx;
	if (elf_getshdrstrndx (elf , &shstrndx) != 0) {
		fprintf(stderr, " elf_getshdrstrndx () failed. ");
		exit(EXIT_FAILURE);
	}

	unsigned int ptrheap = 0;
	bool found_text = false;
	for (size_t i = 0; i < shnum; ++i) {
		Elf_Scn *scn = elf_getscn(elf, i);
		Elf32_Shdr *shdr = elf32_getshdr(scn);

		char *name;
		if ((name = elf_strptr(elf, shstrndx, shdr->sh_name)) == NULL) {
			fprintf(stderr, "elf_strptr() failed.\n");
			exit(EXIT_FAILURE);
		}
		//printf("section %s: addr=%x offset=%x link=%x\n", name, shdr->sh_addr, shdr->sh_offset, shdr->sh_link);

		if		(!strcmp(name, ".text")) {
			found_text = true;
			read_section(mem, elf, i);
			ptrheap = std::max(ptrheap, shdr->sh_addr + shdr->sh_size);
		} 
		else if (!strcmp(name, ".data")) {
			read_section(mem, elf, i);
			ptrheap = std::max(ptrheap, shdr->sh_addr + shdr->sh_size);
		} 
		else if (!strcmp(name, ".bss")) {
			ptrheap = std::max(ptrheap, shdr->sh_addr + shdr->sh_size);
			//	  else if (!strcmp(name, ".symtab"))
			//		symtabndx = i;
		} 
		else if (!strcmp(name, ".rodata")) {
			read_section(mem, elf, i);
			ptrheap = std::max(ptrheap, shdr->sh_addr + shdr->sh_size);
		}
		else if (!strcmp(name, ".strtab")) {
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
	mem.entry(Phead[0].p_vaddr);
	Elf32_Addr e_entry = read_entry(elf);
	Elf32_Addr a_entry = Phead[0].p_vaddr;

	printf("e_entry=%8.8x\n" , e_entry);

	free(Phead);
	// free(bss_p);
	elf_end(elf);
	fclose(fp);
	return a_entry;
}

