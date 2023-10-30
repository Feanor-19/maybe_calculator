# SPU, Assembler & Disassembler

## Build
Repo includes Makefile. Build with `make` in order to build all three programs at once.
Also each program directory (spu, assembler, disassembler) has its own Makefile.

## Usage
### Assembler
Run `assembler.exe`.

#### Available flags:

1. `-h` - prints help.
2. `-i` - specifies input file with program to assemble into binary code (for example, in .txt format).
3. `-o` - specifies output binary file contatining assembled program (for example, in .bin format, it doesn't matter).

#### Available return values:

Please, see `asm_statuses.h`.

### Disassembler
Run `disassembler.exe`.

#### Available flags:

1. `-h` - prints help.
2. `-i` - specifies input file, created by `assembler.exe` (for example, in .bin format, it doesn't matter).
3. `-o` - specifies output text file containing disassembled binary code (for example, in .txt format).

#### Available return values:

Please, see `disasm_statuses.h`.

### SPU (Software Processing Unit)
Run `spu.exe`.

#### Available flags:

1. `-h` - prints help.
2. `-i` - specifies input file, created by `assembler.exe` (for example, in .bin format, it doesn't matter).
3. `-d` - turns on debug mode, in which before executing each command a detailed dump is printed, so the program is executed step by step.

#### Available return values:

Please, see `spu_statuses.h`.

## Key features:
(aside from things, which were must-have and which are not listed here)

1. Code-generating macros (too many of them). Thanks to code-generating macros, adding new command, which doesn't implement new core ideas (and only uses old ones), requires to edit just one file `commands.h`. New command must be described using DSL, which you can find in the beginning of the mentioned file.
2. Customizing. Some changes can be easily done by editing typedefs and constants in `common.h`, but be careful. For example, adding new register to the processor requires just to edit one constant array `registers_names`.
3. RAM. SPU includes RAM, some commands can write to it and read from it. For example, `push [2]` reads third cell and pushes value stored there on stack; `pop [rax]` pops value from stack and writes it to the memory cell by index read from register `rax`.
4. Fixed-point calculations. SPU stack contains integers, but in programs all constants are doubles, just like the result of the program. So, fixed-point calculations connent these things together.
5. Common stack. There is no individual stack for returning addresses of functions, which makes SPU faster and writing programs trickier. See programs `factorial` and `square_solver`!

## Credits
This is MIPT DED32 course project. Thanks to DED32 and my mentor PLT!
