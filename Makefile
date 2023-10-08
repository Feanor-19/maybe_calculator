all: asm dis spu

clean: asm_clean dis_clean spu_clean

.PHONY: dis
dis:
	@make -C .\disassembler

.PHONY: dis_clean
dis_clean:
	@make clean -C .\disassembler

.PHONY: spu
spu:
	@make -C .\spu

.PHONY: spu_clean
spu_clean:
	@make clean -C .\spu

.PHONY: asm
asm:
	@make -C .\assembler

.PHONY: asm_clean
asm_clean:
	@make clean -C .\assembler
