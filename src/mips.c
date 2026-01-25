#include <stdio.h>
#include "mips.h"

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

struct minimu_mips mips_init(void* mem, int len) {
	return (struct minimu_mips) {
		.cpu = (struct minimu_cpu) {
			len,
			0,
			mips_stdin,
			mips_stdout,
			mips_stderr,
		},
		.pc = 0,
		.registers = {},
		.memory = mem
	};
}

// TODO: handle delay slots
// TODO: handle sign and endianness
// TODO: implement coprocessors and doublewords

void mips_execute_syscall(struct minimu_mips *mips, int instr) {
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
		default:
			printf("SYSCALL %02X:", call);
			for (int i = 0; i < 32; i++) {
				printf(" %04X", mips->registers[i]);
			}
			puts("");
	}
}

void mips_execute_r(struct minimu_mips *mips, int instr) {
	int pc = mips->pc;
	int rs = (instr >> 21) & 0x1F;
	int rt = (instr >> 16) & 0x1F;
	int rd = (instr >> 11) & 0x1F;
	int shamt = (instr >> 6) & 0x1F;
	int funct = instr & 0x3F;
	int* rsp = &mips->registers[rs];
	int* rtp = &mips->registers[rt];
	int* rdp = &mips->registers[rd];
	switch (funct) {
		case 0x20: // ADD
			*rdp = *rsp + *rtp; // TODO: signal exception on overflow
			break;
		case 0x21: // ADDU
			*rdp = *rsp + *rtp;
			break;
		case 0x24: // AND
			*rdp = *rsp & *rtp;
			break;
		case 0xD: // BREAK
			// TODO: signal breakpoint exception
			break;
		case 0x1A: // DIV
			mips->registers[32] = *rsp / *rtp;
			mips->registers[33] = *rsp % *rtp;
			break;
		case 0x1B: // DIVU
			mips->registers[32] = *rsp / *rtp; // TODO: handle sign
			mips->registers[33] = *rsp % *rtp;
			break;
		case 0x9: // JALR
			*rdp = mips->pc + 8; // rd = 31
			mips->pc = *rsp; // TODO: handle delay slot
			break;
		case 0x8: // JR
			mips->pc = *rsp; // TODO: handle delay slot
			break;
		case 0x10: // MFHI
			*rdp = mips->registers[33];
			break;
		case 0x12: // MFLO
			*rdp = mips->registers[32];
			break;
		case 0xB: // MOVN
			*rdp = *rtp != 0 ? *rsp : *rdp;
			break;
		case 0xA: // MOVZ
			*rdp = *rtp == 0 ? *rsp : *rdp;
			break;
		case 0x11: // MTHI
			mips->registers[33] = *rsp;
			break;
		case 0x13: // MTLO
			mips->registers[32] = *rsp;
			break;
		case 0x18: // MULT
			mips->registers[32] = (*rsp * *rtp) & 0xFFFF;
			mips->registers[33] = ((long)*rsp * (long)*rtp) >> 32;
			break;
		case 0x19: // MULTU
			mips->registers[32] = (*rsp * *rtp) & 0xFFFF;
			mips->registers[33] = ((long)*rsp * (long)*rtp) >> 32;
			break;
		case 0x27: // NOR
			*rdp = ~(*rsp | *rtp);
			break;
		case 0x25: // OR
			*rdp = *rsp | *rtp;
			break;
		case 0x0: // SLL
			*rdp = *rtp << shamt;
			break;
		case 0x4: // SLLV
			*rdp = *rtp << (*rsp & 0x1F);
			break;
		case 0x2A: // SLT
			*rdp = *rsp < *rtp;
			break;
		case 0x2B: // SLTU
			*rdp = *rsp < *rtp;
			break;
		case 0x3: // SRA
			*rdp = *rtp >> shamt;
			break;
		case 0x7: // SRAV
			*rdp = *rtp >> (*rsp & 0x1F);
			break;
		case 0x2: // SRL
			*rdp = *rtp >> shamt;
			break;
		case 0x6: // SRLV
			*rdp = *rtp >> (*rsp & 0x1F);
			break;
		case 0x22: // SUB
			*rdp = *rsp - *rtp;
			break;
		case 0x23: // SUBU
			*rdp = *rsp - *rtp;
			break;
		case 0xF: // SYNC
			// TODO
			break;
		case 0xC: // SYSCALL
			mips_execute_syscall(mips, instr);
			break;
		case 0x34: // TEQ
			// TODO
			break;
		case 0x30: // TGE
			// TODO
			break;
		case 0x31: // TGEU
			// TODO
			break;
		case 0x32: // TLT
			// TODO
			break;
		case 0x33: // TLTU
			// TODO
			break;
		case 0x36: // TNE
			// TODO
			break;
		case 0x26: // XOR
			*rdp = *rsp ^ *rtp;
			break;
	}
	if (mips->pc == pc) mips->pc += 4;
}

void mips_execute_ri(struct minimu_mips *mips, int instr) {
	int pc = mips->pc;
	int rs = (instr >> 21) & 0x1F;
	int funct = (instr >> 16) & 0x1F;
	int im = instr & 0xFFFF;
	int* rsp = &mips->registers[rs];
	switch (funct) {
		case 0x1: // BGEZ
			mips->pc += (*rsp >= 0 ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0x11: // BGEZAL
			mips->pc += (*rsp >= 0 ? im<<2 : 0); // TODO: handle delay slot
			mips->registers[31] = pc + 4;
			break;
		case 0x13: // BGEZALL
			mips->pc += (*rsp >= 0 ? im<<2 : 0); // TODO: handle delay slot
			mips->registers[31] = pc + 4;
			break;
		case 0x3: // BGEZL
			mips->pc += (*rsp >= 0 ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0x0: // BLTZ
			mips->pc += (*rsp < 0 ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0x10: // BLTZAL
			mips->pc += (*rsp < 0 ? im<<2 : 0); // TODO: handle delay slot
			mips->registers[31] = pc + 4;
			break;
		case 0x12: // BLTZALL
			mips->pc += (*rsp < 0 ? im<<2 : 0); // TODO: handle delay slot
			mips->registers[31] = pc + 4;
			break;
		case 0x2: // BLTZL
			mips->pc += (*rsp < 0 ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0xC: // TEQI
			// TODO
			break;
		case 0x8: // TGEI
			// TODO
			break;
		case 0x9: // TGEIU
			// TODO
			break;
		case 0xA: // TLTI
			// TODO
			break;
		case 0xB: // TLTIU
			// TODO
			break;
		case 0xE: // TNEI
			// TODO
			break;
	}
	if (mips->pc == pc) mips->pc += 4;
}

void mips_execute_j(struct minimu_mips *mips, int instr) {
	int pc = mips->pc;
	int opcode = instr >> 26;
	mips->pc = (pc & (0xF << 28)) | ((instr & ((1 << 26) - 1)) << 2); // TODO: handle delay slot
	if (opcode & 1) {
		// jal; save pc+8 to $ra
		mips->registers[31] = pc + 8;
	}
}

void mips_execute(struct minimu_mips *mips, int flags) {
	int pc = mips->pc;
	int instr = *((int*)(&((char*)mips->memory)[pc]));
	if (flags & 0x2) { // verbose
		int split[6];
		split[0] = instr >> 26;
		split[1] = (instr >> 21) & 0x1F;
		split[2] = (instr >> 16) & 0x1F;
		split[3] = (instr >> 11) & 0x1F;
		split[4] = (instr >> 6) & 0x1F;
		split[5] = instr & 0x3F;
		fprintf(stdout, "%08X: %06b %05b %05b %05b %05b %06b\n", pc, split[0], split[1], split[2], split[3], split[4], split[5]);
	}
	int opcode = instr >> 26;
	mips->registers[0] = 0; // reset $zero
	switch (opcode) {
		case 0x0:
			mips_execute_r(mips, instr); return;
		case 0x1:
			mips_execute_ri(mips, instr); return;
		case 0x2: case 0x3:
			mips_execute_j(mips, instr); return;
	}
	int rs = (instr >> 21) & 0x1F;
	int rt = (instr >> 16) & 0x1F;
	short im = instr & 0xFFFF;
	int* rsp = &mips->registers[rs];
	int* rtp = &mips->registers[rt];
	switch (opcode) {
		case 0x8: // ADDI
			*rtp = *rsp + im; // TODO: signal exception on overflow
			break;
		case 0x9: // ADDIU
			*rtp = *rsp + *((unsigned short*)&im);
			break;
		case 0xC: // ANDI
			*rtp = *rsp & im;
			break;
		case 0x4: // BEQ
			mips->pc += (*rtp == *rsp ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0x14: // BEQL
			mips->pc += (*rtp == *rsp ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0x7: // BGTZ
			mips->pc += (*rsp > 0 ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0x17: // BGTZL
			mips->pc += (*rsp > 0 ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0x6: // BLEZ
			mips->pc += (*rsp < 0 ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0x16: // BLEZL
			mips->pc += (*rsp < 0 ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0x5: // BNE
			mips->pc += (*rtp != *rsp ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0x15: // BNEL
			mips->pc += (*rtp != *rsp ? im<<2 : 0); // TODO: handle delay slot
			break;
		case 0x20: // LB
			*rtp = ((char*)mips->memory)[*rsp + im]; // TODO: vAddr vs pAddr
			break;
		case 0x24: // LBU
			*rtp = ((char*)mips->memory)[*rsp + im]; // TODO: vAddr vs pAddr
			break;
		case 0x21: // LH
			*rtp = ((short*)mips->memory)[*rsp + im]; // TODO: vAddr vs pAddr
			break;
		case 0x25: // LHU
			*rtp = ((short*)mips->memory)[*rsp + im]; // TODO: vAddr vs pAddr
			break;
		case 0x30: // LL
			// TODO
			break;
		case 0xF: // LUI
			*rtp = im<<16;
			break;
		case 0x23: // LW
			*rtp = ((int*)mips->memory)[*rsp + im]; // TODO: vAddr vs pAddr
			break;
		case 0x22: // LWL
			// TODO
			break;
		case 0x26: // LWR
			// TODO
			break;
		case 0x27: // LWU
			*rtp = ((int*)mips->memory)[*rsp + im]; // TODO: vAddr vs pAddr
			break;
		case 0xD: // ORI
			*rtp = *rsp | im;
			break;
		case 0x33: // PREF
			// TODO
			break;
		case 0x28: // SB
			((char*)mips->memory)[*rsp + im] = *rtp & 0xF; // TODO: vAddr vs pAddr
			break;
		case 0x38: // SC
			// TODO
			break;
		case 0x2C: // SBU
			((char*)mips->memory)[*rsp + im] = *rtp & 0xF; // TODO: vAddr vs pAddr
			break;
		case 0x29: // SH
			((short*)mips->memory)[*rsp + im] = *rtp & 0xFF; // TODO: vAddr vs pAddr
			break;
		case 0x2D: // SHU
			((short*)mips->memory)[*rsp + im] = *rtp & 0xFF; // TODO: vAddr vs pAddr
			break;
		case 0xA: // SLTI
			*rtp = *rsp < im;
			break;
		case 0xB: // SLTIU
			*rtp = *rsp < im;
			break;
		case 0x2B: // SW
			((int*)mips->memory)[*rsp + im] = *rtp; // TODO: vAddr vs pAddr
			break;
		case 0x2A: // SWL
			// TODO
			break;
		case 0x2E: // SWR
			// TODO
			break;
		case 0x2F: // SWU
			*rtp = ((int*)mips->memory)[*rsp + im]; // TODO: vAddr vs pAddr
			break;
		case 0xE: // XORI
			*rtp = *rsp ^ im;
			break;
	}
	if (mips->pc == pc) mips->pc += 4; // increment pc, except when explicitly modified
}
