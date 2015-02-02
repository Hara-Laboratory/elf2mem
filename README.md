# elf2mem
Link ELF files and dump the memory.

## Usage
```sh
./elf2mem [options] [-o <output_file>] <input_file> 
```

## Example
```sh
elf2mem -s 4 -n MEM -o aout.h -b 0x01000 -e 0x0e000 a.out
```
It outputs memory from 0x1000 to 0xe000, scattering each byte into four files.
In the end, a array in each file will have 0x340 elements.

## Options
| Option          | Description                                            |
| --------------- | ------------------------------------------------------ |
| `-b` _addr_  		*or* `--start-address` _addr_ | Start address.                                         |
| `-B`            	*or* `--start-minimal`		| Output from the first section. (default)               |
| `-e` _addr_  		*or* `--end-address` _addr_   | End address.                                           |
| `-E`            	*or* `--end-minimal` 			| Output until end of the sections. (default)            |
| `-t` _type_  		*or* `--type` _type_   		| Output type. <br> Available choices: `c-array` (default) |
| `-o` _filename_ 	*or* `--output` _filename_	| Output file name (scheme).                             |
| `-s` _num_      	*or* `--split` _num_ 			| Split number.                                          |
| `-w` _num_      	*or* `--bit-width` _num_ 		| Output bit-width.                                      |
| `--byte-order` _order_ 						| Output byte-order. <br> Available choices: `bit-endian` (default), `little-endian` |

### Options for output type `c-array`:
| Option          | Description                                            |
| --------------- | ------------------------------------------------------ |
| `-n` _name_      	*or* `--identifier` _name_ 	| Output identifier name.  |

## Dependencies
- libelf
- getopt

## Supporting sections
- `.text`
- `.data`
- `.rodata`
- `.bss` (nothing to do for support ;)
