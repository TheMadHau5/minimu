#pragma once
#ifndef MINIMU_MIPS_DBG_H
#define MINIMU_MIPS_DBG_H

#include "../common.h"
#include "mips.h"

#define MINIMU_DEBUGGER_MAX_BREAKPOINTS 64

typedef struct {
	uint64_t addresses[MINIMU_DEBUGGER_MAX_BREAKPOINTS];
	size_t count;
} minimu_debug_t;

typedef enum {
	MINIMU_DEBUG_STOP_HALTED,
	MINIMU_DEBUG_STOP_BREAKPOINT,
	MINIMU_DEBUG_STOP_STEP,
	MINIMU_DEBUG_STOP_QUIT,
} minimu_debug_stop_t;

void minimu_debug_init(minimu_debug_t *dbg);

bool minimu_debug_add_breakpoint(minimu_debug_t *dbg, uint64_t addr);
bool minimu_debug_remove_breakpoint(minimu_debug_t *dbg, uint64_t addr);
bool minimu_debug_has_breakpoint(const minimu_debug_t *dbg, uint64_t addr);

bool minimu_debug_step(struct minimu_mips *mips, uint64_t mem_size);
void minimu_debug_repl(struct minimu_mips *mips, minimu_debug_t *dbg, uint64_t mem_size);

void minimu_debug_print_registers(const struct minimu_mips *mips);
void minimu_debug_print_stack(const struct minimu_mips *mips, uint64_t mem_size, int count);
void minimu_debug_print_memory(const struct minimu_mips *mips, uint64_t mem_size, uint64_t addr, uint64_t len);

#endif // MINIMU_MIPS_DBG_H
