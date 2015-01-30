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

## Options
| Option          | Description                                            |
| --------------- | ------------------------------------------------------ |
| `-b` _addr_     | Start address.                                         |
| `-B`            | Output from the first section. (default)               |
| `-e` _addr_     | End address.                                           |
| `-E`            | Output until end of the sections. (default)            |
| `-t` _type_     | Output type. <br> Available choices: `c-array` (default) |
| `-o` _filename_ | Output file name (scheme).                             |
| `-s` _num_      | Split number.                                          |

### Options for output type `c-array`:
| Option          | Description                                            |
| --------------- | ------------------------------------------------------ |
| `-n` _name_     | Identifier name.                                       |

## Dependencies
- libelf
- getopt
