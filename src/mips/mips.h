#pragma once
#ifndef MINIMU_MIPS_H
#define MINIMU_MIPS_H

#include "../common.h"

struct minimu_mips {
	struct minimu_cpu cpu;
	uint64_t registers[32];
	uint64_t special[3]; // PC, LO, HI
	uint64_t pipeline[4][4]; // INSTR, PC+4, REG1, REG2, ALU, Mem
	uint64_t pipeline_ifid[2]; // INSTR, PC+4
	uint64_t pipeline_idex[5]; // INSTR, PC+4, REG1, REG2, IMM
	uint64_t pipeline_exmem[3]; // INSTR, REG2, ALU
	uint64_t pipeline_memwb[3]; // INSTR, ALU, MEM
	void* memory;
};

struct minimu_mips_cop {
	uint64_t registers[32];
	void* memory;
};

struct minimu_mips mips_init(void*, uint32_t, uint16_t);
void mips_disasm(uint32_t);
void mips_execute_syscall(struct minimu_mips *, uint32_t);

#endif // MINIMU_MIPS_H
