#include <stdlib.h>
#include <string.h>
#include <libelf.h>
#include <gelf.h>
#include <iostream>
#include <sstream>
#include <string>
#include "readerElf.h"
#include "memchunk.h"
#include "byteorder.h"

#define forcenonnull(ptr,...) errorif((ptr)==NULL,__VA_ARGS__)
#define errorif(...) errorif2(__VA_ARGS__,0)
#define errorif2(test,fmt,...) do{if(test){fprintf(stderr, "%s:%s:%d: " fmt "\n",__FILE__, __func__, __LINE__, __VA_ARGS__);exit(EXIT_FAILURE);}}while(0)

static const std::string keyname_version = "version";
static const std::string keyname_byteorder = "byte-order";
static const std::string keyname_wordlength = "word-size";

using namespace memory;

static const int supported_version_upper = 1;
static const int supported_version_lower = 1;

enum SubleqObjSectionType {
    OBJ_NULL,
    KEYVALUE,
    MEMORY
};

static void skip_section(std::istream &ism) {
    std::string str;
    while (std::getline(ism, str)) {
	if (!str.compare(""))
	    break;
    }
}

static std::map<std::string, std::string> *read_keyvalue_section(std::istream &ism) {
    auto sec = new std::map<std::string, std::string>();
    std::string str;
    while (std::getline(ism, str)) {
	if (!str.compare(""))
	    break;
	auto key_start = str.find_first_not_of(" \t");
	auto key_end = str.find_first_of(":");
	auto value_start = str.find_first_not_of(" \t", key_end + 1);
	std::string key = str.substr(key_start, key_end - key_start);
	std::string value = str.substr(value_start);
	sec->insert(std::make_pair(key, value));
    }

    return sec;
}

static std::string make_chunk_name(const std::string &name, size_t addr) {
    std::ostringstream ostrs;
    ostrs << name << "@" << std::hex << addr;
    auto && str = ostrs.str();
    return std::move(str);
}

static bool read_memory_section(std::istream &ism, Memory &mem, byteorder order, const std::string &name) {

    unsigned int addr = 0;
    std::vector<unsigned char> v;
    std::string str;
    std::string chunk_name;
    while (std::getline(ism, str)) {
	if (!str.compare(""))
	    break;
	
	auto addr_start = str.find_first_of("@");
	auto addr_end = str.find_first_of(":");
	auto value_start = addr_end == std::string::npos ? 0 : addr_end + 1;
	if (addr_start != std::string::npos) {
	    if (v.size() != 0) {
		// std::cout << "append: " << addr << std::endl;
		Chunk ch(chunk_name, v);
		mem.addChunk(order.size() * addr, ch);
		v.clear();
	    }
	    auto addr_str = str.substr(addr_start + 1, addr_end - addr_start - 1);
	    addr = std::stoi(addr_str);
	    chunk_name = make_chunk_name(name, addr);
	}
	auto value_str = str.substr(value_start);
	std::istringstream value_s(value_str);

	unsigned int value;
	while (value_s >> value) {
	    for (int i = 0; i < order.size(); ++i) {
		unsigned char ch = value >> (8 * (order.size() - order.index(i) - 1));
		v.push_back(ch);
	    }
	}
    }

    // std::cout << "append: " << addr << std::endl;
    Chunk ch(chunk_name, v);
    mem.addChunk(order.size() * addr, ch);

    return true;
}

static SubleqObjSectionType default_section_type(std::string &str) {
    if (!str.compare("header") || !str.compare("symbols")) {
	return KEYVALUE;
    } else if (!str.compare("symbols")) {
	return MEMORY;
    }
    return OBJ_NULL;
}

static std::pair<std::string, SubleqObjSectionType> read_section_name(std::istream &ism) {
    std::string str;
    if (std::getline(ism, str)) {
	auto name_start = str.find_first_not_of('[');
	auto name_end = str.find_last_of(":]");

	if (name_start == std::string::npos || name_end == std::string::npos) {
	    std::cerr << "Section header '" << str << "' is illegal format.\n";
	    exit(EXIT_FAILURE);
	}
	// std::cout << str << std::endl;
	// std::cout << name_start << ", " << name_end << std::endl;

	auto header_name = str.substr(name_start, name_end - name_start);
	return std::make_pair(header_name, default_section_type(header_name));
    } else {
	std::cerr << "No section header.\n";
	exit(EXIT_FAILURE);
    }
}

void read_extra(Memory &mem, std::istream &is, const std::string &name = "")
{
    auto header_name = read_section_name(is);

    // std::cout << ":" << header_name.first << ": " << static_cast<int>(header_name.second) << std::endl;

    auto header = read_keyvalue_section(is);
    
    /*
    for (std::map<std::string,std::string>::iterator it = header->begin(); it != header->end(); ++it)
	std::cout << it->first << ": " << it->second << std::endl;
    */

    int version = std::stoi((*header)[keyname_version]);
    if (version < supported_version_lower || version > supported_version_upper) {
	std::cerr << "File format version " << version << " is not supported by this program. Only " << supported_version_lower << " to " << supported_version_upper << "." << std::endl;
    }

    if (!header->count(keyname_byteorder)) {
	std::cerr << "Byte order is not given. I assume \"big-endian\"." << std::endl;
	(*header)[keyname_byteorder] = "big-endian";
    }

    if (!header->count(keyname_wordlength)) {
	std::cerr << "Word length is not given. I assume 4 octets." << std::endl;
	(*header)[keyname_wordlength] = "4";
    }
    auto length = std::stoi((*header)[keyname_wordlength]);

    byteorder order = byteorder::from_string((*header)[keyname_byteorder], length);

    while (true) {
	auto section = read_section_name(is);

	if (!section.first.compare("text")) {
	    break;
	} else {
	    skip_section(is);
	}
    }

    read_memory_section(is, mem, order, name);
}
// vim: set noet fenc=utf-8 ff=unix sts=0 sw=4 ts=8 : 
