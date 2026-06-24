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
		.registers = {},
		.special = {},
		.pipeline = {},
		.memory = mem
	};
}

// TODO: handle delay slots
// TODO: handle sign and endianness
// TODO: implement coprocessors and doublewords

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

// TODO: handle endianness
// TODO: handle bounds
uint32_t mips_get_memword(struct minimu_mips* mips, uint64_t loc) {
	uint32_t val;
	memcpy(&val, &mips->memory[loc], sizeof(val));
	return val;
}

uint64_t mips_get_memdword(struct minimu_mips* mips, uint64_t loc) {
	uint64_t val;
	memcpy(&val, &mips->memory[loc], sizeof(val));
	return val;
}

void mips_set_memword(struct minimu_mips* mips, uint64_t loc, uint32_t val) {
	memcpy(&mips->memory[loc], &val, sizeof(val));
}

void mips_set_memdword(struct minimu_mips* mips, uint64_t loc, uint64_t val) {
	memcpy(&mips->memory[loc], &val, sizeof(val));
}

void mips_pipeline_cycle(struct minimu_mips* mips) {
	/*
	uint64_t pipeline[4][4]; // next

	// WB
	uint8_t rt = (mips->pipeline[3][0] >> 16) & 0x1F;
	mips->registers[rt] = mips->pipeline[3][1];

	// MEM
	pipeline[3][0] = mips->pipeline[2][0];
	pipeline[3][1] = mips->pipeline[2][1];
	pipeline[3][2] = mips->pipeline[2][2];
	pipeline[3][3] = mips->pipeline[2][3];

	// EX
	pipeline[2][0] = mips->pipeline[1][0];
	pipeline[2][1] = mips->pipeline[1][1];
	pipeline[2][2] = mips->pipeline[1][2];
	pipeline[2][3] = mips->pipeline[1][3];
	uint8_t instr = pipeline[2][0] >> 26;
	if (instr == 0) {
		// SPECIAL
		instr = 0x80;
		instr |= pipeline[2][0] & 0x3F;
	} else if (instr == 1) {
		// REGIMM
		instr = 0xC0;
		instr |= (pipeline[2][0] >> 16) & 0x1F;
	}
	pipeline[2][2] = mips_execute_alu(mips, instr, pipeline[2][2], pipeline[2][3]);
	// TODO: flush upcoming IF
	if (0) {

		pipeline[0][0] = mips_get_memword(mips, mips->special[0]);
		pipeline[0][1] = mips->special[0] + 4;
		mips->special[0] = pipeline[0][1];
	}

	// ID
	pipeline[1][0] = mips->pipeline[0][0];
	uint8_t rs = (pipeline[1][0] >> 21) & 0x1F;
	rt = (pipeline[1][0] >> 16) & 0x1F;
	int16_t imm = pipeline[1][0] & 0xFF;
	pipeline[1][1] = mips->pipeline[0][1] + (((int64_t)imm) << 2);
	pipeline[1][2] = mips->registers[rs];
	pipeline[1][3] = mips->registers[rt];
	// TODO: resolve branch here to avoid wasting one cycle

	// IF
	// TODO: branch predictor
	pipeline[0][0] = mips_get_memword(mips, mips->special[0]);
	pipeline[0][1] = mips->special[0] + 4;
	mips->special[0] = pipeline[0][1];

	// TODO: handle exceptions

	// cycle forward
	memcpy(mips->pipeline, pipeline, sizeof(pipeline));
	*/

}
