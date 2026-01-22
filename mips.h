#include "common.h"

struct minimu_mips {
	struct minimu_cpu cpu;
	int pc;
	int registers[34]; // 32 = LO, 33 = HI
	void* memory;
};

void mips_execute(struct minimu_mips *);
