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

Address is a value in _byte addressing_, but **not** word addressing.

## Options
| Option          | Description                                            |
| --------------- | ------------------------------------------------------ |
| `-b` _addr_  		*or* `--start-address` _addr_ | Start address. (*byte addressing*)                                        |
| `-B`            	*or* `--start-minimal`		| Output from the first section. (default)               |
| `-e` _addr_  		*or* `--end-address` _addr_   | End address. (*byte addressing*)                                          |
| `-E`            	*or* `--end-minimal` 			| Output until end of the sections. (default)            |
| `-t` _type_  		*or* `--type` _type_   		| Output type. <br> Available choices: `c-array` (default) |
| `-o` _filename_ 	*or* `--output` _filename_	| Output file name (scheme).                             |
| `-s` _num_      	*or* `--split` _num_ 			| Split number.                                          |
| `-w` _num_      	*or* `--bit-width` _num_ 		| Output bit-width. <br> Available choices: 8 (default), 16, 32, 64 |
| `--byte-order` _order_ 						| Output byte-order. <br> Available choices: `big-endian` (default), `little-endian` |
| `-x` _filename_  	*or* `--extra` _filename_ 		| Extra input file with special format. (multple occasion is allowed) |

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

## Example of Extra Input
All addresses is prefexed with `@` and word addressing.

```
[header]
    version: 1
    byte-order: big-endian
    word-size: 4
    type: packed
    end: 208

[symbols]
    add: @193
    mult: @100

[text]
    @100: 6 1 118 1 6 106 6 10 109 6 10 112 6 6 115 4 9 118 1 10 121 5 9 124 6 2 142 2 6 130 6 11 133 6 11 136 6 6 139 4 9 142 2 11 145 5 9 148 288 288 151 5 10 157 11 288 151 6 9 169 9 9 163 10 10 166 6 6 -1 10 10 172 288 10 175 11 11 178 10 11 181 288 288 184 11 288 187 4 9 169 6 4 160 2 6 196 3 6 199 1 1 202 6 1 205 6 6 -1
```
