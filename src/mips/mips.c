#include "mips.h"
#include <stdint.h>

void mips_execute(struct minimu_cpu* mips_cpu);

char* mips_stdin() {
	static char line[256];
	return fgets(line, sizeof(line), stdin);
}
void mips_stdout(char* str) {
	printf("mips stdout: %s\n", str);
}
void mips_stderr(char* str) {
	printf("mips stderr: %s\n", str);
}

struct minimu_mips mips_init(void* mem, uint32_t len, uint16_t flags) {
	return (struct minimu_mips) {
		.cpu = (struct minimu_cpu) {
			&mips_execute,
			len,
			flags,
			0,
		},
		.registers = {0},
		.fpu_regs = {0},
		.special = {0},
		.fcsr = 0,
		.has_pending_branch = false,
		.delay_slot_annul = false,
		.pending_branch_target = 0,
		.memory = mem
	};
}

void mips_execute_syscall(struct minimu_mips *mips, uint32_t instr) {
	// currently just dumps regs
	int call = mips->registers[2];
	int cval = mips->registers[4];
	switch (call) {
		case 0x1:
			printf("%d", cval);
			break;
		case 0x4:
			printf("%s", &((char*)mips->memory)[cval]);
			break;
		case 0xA:
			printf("HALT\n");
			break;
		default:
			printf("SYSCALL %02X:", call);
			for (int i = 0; i < 32; i++) {
				printf(" %04lX", mips->registers[i]);
			}
			puts("");
	}
}

uint32_t mips_get_memword(struct minimu_mips* mips, uint64_t loc, bool *ok) {
	if (loc + 4 > mips->cpu.psize) { if (ok) *ok = false; return 0; }
	uint8_t b[4];
	memcpy(b, &((uint8_t*)mips->memory)[loc], 4);
	if (ok) *ok = true;
	return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

uint64_t mips_get_memdword(struct minimu_mips* mips, uint64_t loc, bool *ok) {
	if (loc + 8 > mips->cpu.psize) { if (ok) *ok = false; return 0; }
	uint8_t b[8];
	memcpy(b, &((uint8_t*)mips->memory)[loc], 8);
	if (ok) *ok = true;
	uint64_t val = 0;
	for (int i = 7; i >= 0; i--) val = (val << 8) | b[i];
	return val;
}

void mips_set_memword(struct minimu_mips* mips, uint64_t loc, uint32_t val, bool *ok) {
	if (loc + 4 > mips->cpu.psize) { if (ok) *ok = false; return; }
	uint8_t b[4] = {
		(uint8_t)(val & 0xff), (uint8_t)((val >> 8) & 0xff),
		(uint8_t)((val >> 16) & 0xff), (uint8_t)((val >> 24) & 0xff),
	};
	memcpy(&((uint8_t*)mips->memory)[loc], b, 4);
	if (ok) *ok = true;
}

void mips_set_memdword(struct minimu_mips* mips, uint64_t loc, uint64_t val, bool *ok) {
	if (loc + 8 > mips->cpu.psize) { if (ok) *ok = false; return; }
	uint8_t b[8];
	for (int i = 0; i < 8; i++) b[i] = (uint8_t)((val >> (8 * i)) & 0xff);
	memcpy(&((uint8_t*)mips->memory)[loc], b, 8);
	if (ok) *ok = true;
}

void mips_apply_pending_branch(struct minimu_mips *mips) {
	if (!mips->has_pending_branch) return;
	mips->special[0] = mips->pending_branch_target;
	mips->has_pending_branch = false;
	mips->delay_slot_annul = false;
}
