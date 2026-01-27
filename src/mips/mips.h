#pragma once
#ifndef MINIMU_MIPS_H
#define MINIMU_MIPS_H

#include "../common.h"

struct minimu_mips {
	struct minimu_cpu cpu;
	uint32_t pc;
	uint32_t registers[34]; // 32 = LO, 33 = HI
	void* memory;
};

struct minimu_mips mips_init(void*, uint32_t, uint16_t);

#endif // MINIMU_MIPS_H
