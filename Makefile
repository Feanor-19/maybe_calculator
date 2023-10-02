all: asm dis spu

.PHONY: asm
asm:
	@make -f Makefile_assembler

.PHONY: asm_clean
asm_clean:
	@make -f Makefile_assembler clean

.PHONY: dis
dis:
	@make -f Makefile_disassembler

.PHONY: dis_clean
dis_clean:
	@make -f Makefile_disassembler clean

.PHONY: spu
spu:
	@make -f Makefile_spu

.PHONY: spu_clean
spu_clean:
	@make -f Makefile_spu clean

.PHONY: clean
clean: asm_clean dis_clean spu_clean
