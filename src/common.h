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

#define MINIMU_STATUS_RUNNING       0
#define MINIMU_STATUS_OOB           1
#define MINIMU_STATUS_DIV_BY_ZERO   2
#define MINIMU_STATUS_INVALID_OP    3
#define MINIMU_STATUS_OVERFLOW      4
#define MINIMU_STATUS_BREAKPOINT    5
#define MINIMU_STATUS_SYSCALL_EXIT  6

#endif // MINIMU_COMMON_H
