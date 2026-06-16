#pragma once
#ifndef MINIMU_COMMON_H
#define MINIMU_COMMON_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

struct minimu_cpu {
	void (*execute)(struct minimu_cpu *);
	uint64_t psize;
	uint16_t flags;
	uint8_t status;
};

#endif // MINIMU_COMMON_H
