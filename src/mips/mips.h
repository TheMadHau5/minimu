#pragma once
#ifndef MINIMU_MIPS_H
#define MINIMU_MIPS_H

#include "../common.h"

// TODO: impl cop0
struct minimu_mips {
	struct minimu_cpu cpu;
	uint64_t registers[32];
	uint64_t special[3]; // PC, LO, HI
	uint64_t fpu_regs[32];
	uint64_t fcsr;

	bool has_pending_branch;
	bool delay_slot_annul;
	uint64_t pending_branch_target;

	void* memory;
};

typedef struct {
	char* mnemonic;
	uint8_t opcode;
	uint8_t funct; // also used for REGIMM, COPz, etc.
	uint16_t control;
} minimu_mips_instr_t;

struct minimu_mips mips_init(void*, uint32_t, uint16_t);
void mips_disasm(uint32_t);
void mips_execute_syscall(struct minimu_mips *, uint32_t);
void mips_apply_pending_branch(struct minimu_mips *mips);
void minimu_mips_fpu_execute(struct minimu_mips *mips, uint32_t instr);

uint32_t mips_get_memword(struct minimu_mips* mips, uint64_t loc, bool *ok);
uint64_t mips_get_memdword(struct minimu_mips* mips, uint64_t loc, bool *ok);
void mips_set_memword(struct minimu_mips* mips, uint64_t loc, uint32_t val, bool *ok);
void mips_set_memdword(struct minimu_mips* mips, uint64_t loc, uint64_t val, bool *ok);

#endif // MINIMU_MIPS_H
