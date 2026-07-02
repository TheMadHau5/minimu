#include "dbg.h"
#include <ctype.h>
#include <inttypes.h>
#include <strings.h>

static const char *const minimu_reg_names[32] = {
	"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
	"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
	"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
	"$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra",
};

void minimu_debug_init(minimu_debug_t *dbg) {
	dbg->count = 0;
	for (size_t i = 0; i < MINIMU_DEBUGGER_MAX_BREAKPOINTS; i++) {
		dbg->addresses[i] = 0;
	}
}

bool minimu_debug_add_breakpoint(minimu_debug_t *dbg, uint64_t addr) {
	if (minimu_debug_has_breakpoint(dbg, addr)) return true;
	if (dbg->count >= MINIMU_DEBUGGER_MAX_BREAKPOINTS) return false;
	dbg->addresses[dbg->count++] = addr;
	return true;
}

bool minimu_debug_remove_breakpoint(minimu_debug_t *dbg, uint64_t addr) {
	for (size_t i = 0; i < dbg->count; i++) {
		if (dbg->addresses[i] == addr) {
			dbg->addresses[i] = dbg->addresses[dbg->count - 1];
			dbg->count--;
			return true;
		}
	}
	return false;
}

bool minimu_debug_has_breakpoint(const minimu_debug_t *dbg, uint64_t addr) {
	for (size_t i = 0; i < dbg->count; i++) {
		if (dbg->addresses[i] == addr) return true;
	}
	return false;
}

static bool pc_in_bounds(const struct minimu_mips *mips, uint64_t mem_size) {
	return mips->special[0] + 4 <= mem_size;
}

bool minimu_debug_step(struct minimu_mips *mips, uint64_t mem_size) {
	if (mips->cpu.status != 0) return false;
	if (!pc_in_bounds(mips, mem_size)) {
		mips->cpu.status = 1;
		return false;
	}
	mips->cpu.execute(&mips->cpu);
	return true;
}

void minimu_debug_print_registers(const struct minimu_mips *mips) {
	printf("pc  = 0x%016" PRIx64 "   lo  = 0x%016" PRIx64 "   hi  = 0x%016" PRIx64 "\n",
		mips->special[0], mips->special[1], mips->special[2]);
	for (int i = 0; i < 32; i += 2) {
		printf("r%-2d %-5s = 0x%016" PRIx64 "   r%-2d %-5s = 0x%016" PRIx64 "\n",
			i, minimu_reg_names[i], mips->registers[i],
			i + 1, minimu_reg_names[i + 1], mips->registers[i + 1]);
	}
}

void minimu_debug_print_stack(const struct minimu_mips *mips, uint64_t mem_size, int count) {
	uint64_t sp = mips->registers[29];
	printf("stack (from $sp = 0x%016" PRIx64 "):\n", sp);
	for (int i = 0; i < count; i++) {
		uint64_t addr = sp + (uint64_t)i * 8;
		if (addr + 8 > mem_size) {
			printf("  0x%016" PRIx64 ": <out of bounds>\n", addr);
			continue;
		}
		uint64_t val;
		memcpy(&val, &((char *)mips->memory)[addr], sizeof(val));
		printf("  0x%016" PRIx64 ": 0x%016" PRIx64 "%s\n", addr, val, addr == sp ? "  <- $sp" : "");
	}
}

void minimu_debug_print_memory(const struct minimu_mips *mips, uint64_t mem_size, uint64_t addr, uint64_t len) {
	if (addr >= mem_size) {
		printf("address 0x%016" PRIx64 " is out of bounds (memory size 0x%016" PRIx64 ")\n", addr, mem_size);
		return;
	}
	if (addr + len > mem_size) len = mem_size - addr;

	const uint8_t *bytes = (const uint8_t *)mips->memory;
	for (uint64_t off = 0; off < len; off += 16) {
		printf("  0x%016" PRIx64 ": ", addr + off);
		uint64_t row = (len - off < 16) ? (len - off) : 16;
		for (uint64_t i = 0; i < 16; i++) {
			if (i < row) printf("%02X ", bytes[addr + off + i]);
			else printf("   ");
			if (i == 7) printf(" ");
		}
		printf(" ");
		for (uint64_t i = 0; i < row; i++) {
			uint8_t b = bytes[addr + off + i];
			putchar(isprint(b) ? (char)b : '.');
		}
		printf("\n");
	}
}

static void print_help(void) {
	printf(
		"debugger commands:\n"
		"  step, s [n]        execute n instructions (default 1)\n"
		"  continue, c        run until a breakpoint or halt\n"
		"  run, r             alias for continue\n"
		"  break, b <addr>    set a breakpoint at addr (hex, e.g. 0x400)\n"
		"  delete, d <addr>   remove a breakpoint at addr\n"
		"  list, l            list current breakpoints\n"
		"  registers, regs    show all registers\n"
		"  print, p $reg      show a single register ($pc, $lo, $hi, $0-$31 or names)\n"
		"  stack [n]          show n words of stack from $sp (default 8)\n"
		"  memory, m <addr> [len]  show len bytes at addr (hex, default 64)\n"
		"  help, h            show this message\n"
		"  quit, q            leave the debugger (program keeps whatever state it has)\n"
	);
}

static bool parse_hex_or_dec(const char *s, uint64_t *out) {
	if (!s || !*s) return false;
	char *end = NULL;
	uint64_t v = strtoull(s, &end, 0);
	if (end == s) return false;
	*out = v;
	return true;
}

static int reg_index_from_name(const char *tok) {
	if (!tok) return -1;
	if (tok[0] == '$') tok++;
	if (isdigit((unsigned char)tok[0])) {
		char *end = NULL;
		long v = strtol(tok, &end, 10);
		if (*end == '\0' && v >= 0 && v < 32) return (int)v;
		return -1;
	}
	char buf[8] = {0};
	snprintf(buf, sizeof(buf), "$%s", tok);
	for (int i = 0; i < 32; i++) {
		if (strcasecmp(buf, minimu_reg_names[i]) == 0) return i;
	}
	return -1;
}

static void handle_print(const struct minimu_mips *mips, const char *arg) {
	if (!arg) {
		printf("usage: print $reg\n");
		return;
	}
	if (strcasecmp(arg, "$pc") == 0 || strcasecmp(arg, "pc") == 0) {
		printf("$pc = 0x%016" PRIx64 "\n", mips->special[0]);
		return;
	}
	if (strcasecmp(arg, "$lo") == 0 || strcasecmp(arg, "lo") == 0) {
		printf("$lo = 0x%016" PRIx64 "\n", mips->special[1]);
		return;
	}
	if (strcasecmp(arg, "$hi") == 0 || strcasecmp(arg, "hi") == 0) {
		printf("$hi = 0x%016" PRIx64 "\n", mips->special[2]);
		return;
	}
	int idx = reg_index_from_name(arg);
	if (idx < 0) {
		printf("unknown register '%s'\n", arg);
		return;
	}
	printf("%s = 0x%016" PRIx64 " (%" PRId64 ")\n", minimu_reg_names[idx], mips->registers[idx], (int64_t)mips->registers[idx]);
}

static void announce_stop(minimu_debug_stop_t why, uint64_t pc) {
	switch (why) {
		case MINIMU_DEBUG_STOP_HALTED:
			printf("program halted at pc = 0x%016" PRIx64 "\n", pc);
			break;
		case MINIMU_DEBUG_STOP_BREAKPOINT:
			printf("breakpoint hit at pc = 0x%016" PRIx64 "\n", pc);
			break;
		case MINIMU_DEBUG_STOP_STEP:
			printf("stopped at pc = 0x%016" PRIx64 "\n", pc);
			break;
		case MINIMU_DEBUG_STOP_QUIT:
			break;
	}
}

static minimu_debug_stop_t run_until_stop(struct minimu_mips *mips, minimu_debug_t *dbg, uint64_t mem_size) {
	uint64_t start_pc = mips->special[0];
	bool first = true;
	while (mips->cpu.status == 0) {
		if (!first && minimu_debug_has_breakpoint(dbg, mips->special[0])) {
			return MINIMU_DEBUG_STOP_BREAKPOINT;
		}
		first = false;
		(void)start_pc;
		if (!minimu_debug_step(mips, mem_size)) {
			return MINIMU_DEBUG_STOP_HALTED;
		}
	}
	return MINIMU_DEBUG_STOP_HALTED;
}

void minimu_debug_repl(struct minimu_mips *mips, minimu_debug_t *dbg, uint64_t mem_size) {
	char line[256];
	printf("minimu debugger. type 'help' for commands.\n");

	for (;;) {
		if (mips->cpu.status != 0) {
			printf("(program halted, pc = 0x%016" PRIx64 ") > ", mips->special[0]);
		} else {
			printf("(minimu) [pc=0x%08" PRIx64 "] > ", mips->special[0]);
		}
		if (!fgets(line, sizeof(line), stdin)) {
			printf("\n");
			return;
		}
		char *cmd = strtok(line, " \t\r\n");
		if (!cmd) continue;
		char *arg1 = strtok(NULL, " \t\r\n");
		char *arg2 = strtok(NULL, " \t\r\n");

		if (strcmp(cmd, "step") == 0 || strcmp(cmd, "s") == 0) {
			uint64_t n = 1;
			if (arg1) parse_hex_or_dec(arg1, &n);
			if (mips->cpu.status != 0) {
				printf("program already halted\n");
				continue;
			}
			uint64_t i = 0;
			for (; i < n; i++) {
				if (!minimu_debug_step(mips, mem_size)) break;
				if (minimu_debug_has_breakpoint(dbg, mips->special[0])) { i++; break; }
			}
			announce_stop(mips->cpu.status != 0 ? MINIMU_DEBUG_STOP_HALTED : MINIMU_DEBUG_STOP_STEP, mips->special[0]);
		} else if (strcmp(cmd, "continue") == 0 || strcmp(cmd, "c") == 0 ||
		           strcmp(cmd, "run") == 0 || strcmp(cmd, "r") == 0) {
			if (mips->cpu.status != 0) {
				printf("program already halted\n");
				continue;
			}
			minimu_debug_stop_t why = run_until_stop(mips, dbg, mem_size);
			announce_stop(why, mips->special[0]);
		} else if (strcmp(cmd, "break") == 0 || strcmp(cmd, "b") == 0) {
			uint64_t addr;
			if (!arg1 || !parse_hex_or_dec(arg1, &addr)) {
				printf("usage: break <addr>\n");
				continue;
			}
			if (minimu_debug_add_breakpoint(dbg, addr)) {
				printf("breakpoint set at 0x%016" PRIx64 "\n", addr);
			} else {
				printf("could not set breakpoint (limit reached)\n");
			}
		} else if (strcmp(cmd, "delete") == 0 || strcmp(cmd, "d") == 0) {
			uint64_t addr;
			if (!arg1 || !parse_hex_or_dec(arg1, &addr)) {
				printf("usage: delete <addr>\n");
				continue;
			}
			if (minimu_debug_remove_breakpoint(dbg, addr)) {
				printf("breakpoint removed at 0x%016" PRIx64 "\n", addr);
			} else {
				printf("no breakpoint at 0x%016" PRIx64 "\n", addr);
			}
		} else if (strcmp(cmd, "list") == 0 || strcmp(cmd, "l") == 0) {
			if (dbg->count == 0) {
				printf("no breakpoints set\n");
			} else {
				for (size_t i = 0; i < dbg->count; i++) {
					printf("  [%zu] 0x%016" PRIx64 "\n", i, dbg->addresses[i]);
				}
			}
		} else if (strcmp(cmd, "registers") == 0 || strcmp(cmd, "regs") == 0) {
			minimu_debug_print_registers(mips);
		} else if (strcmp(cmd, "print") == 0 || strcmp(cmd, "p") == 0) {
			handle_print(mips, arg1);
		} else if (strcmp(cmd, "stack") == 0) {
			int count = 8;
			if (arg1) {
				uint64_t v;
				if (parse_hex_or_dec(arg1, &v)) count = (int)v;
			}
			minimu_debug_print_stack(mips, mem_size, count);
		} else if (strcmp(cmd, "memory") == 0 || strcmp(cmd, "m") == 0) {
			uint64_t addr, len = 64;
			if (!arg1 || !parse_hex_or_dec(arg1, &addr)) {
				printf("usage: memory <addr> [len]\n");
				continue;
			}
			if (arg2) parse_hex_or_dec(arg2, &len);
			minimu_debug_print_memory(mips, mem_size, addr, len);
		} else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0) {
			print_help();
		} else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
			return;
		} else {
			printf("unknown command '%s' (try 'help')\n", cmd);
		}
	}
}
