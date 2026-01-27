#include <stdio.h>

#include "mips.h"

const char *const regs[32] = {
	"$zero",
	"$at",
	"$v0",
	"$v1",
	"$a0",
	"$a1",
	"$a2",
	"$a3",
	"$t0",
	"$t1",
	"$t2",
	"$t3",
	"$t4",
	"$t5",
	"$t6",
	"$t7",
	"$s0",
	"$s1",
	"$s2",
	"$s3",
	"$s4",
	"$s5",
	"$s6",
	"$s7",
	"$t8",
	"$t9",
	"$k0",
	"$k1",
	"$gp",
	"$sp",
	"$fp",
	"$ra"
};


void mips_disasm(uint32_t instr) {
	uint8_t opcode = instr >> 26;
	uint8_t rs = (instr >> 21) & 0x1F;
	uint8_t rt = (instr >> 16) & 0x1F;
	uint16_t im = instr & 0xFFFF;
	char* mnemonic = NULL;
	int imval = im;
	switch (opcode) {
		case 0x00:
			// TODO: mips_disasm_r(instr);
			// return;
			mnemonic = "SPECIAL";
			break;
		case 0x01:
			// TODO: mips_disasm_ri(instr);
			// return;
			mnemonic = "REGIMM";
			break;
		case 0x02:
			mnemonic = "J"; // TODO: j type
			break;
		case 0x03:
			mnemonic = "JAL"; // TODO: j type
			break;
		case 0x04:
			mnemonic = "BEQ";
			break;
		case 0x05:
			mnemonic = "BNE";
			break;
		case 0x06:
			mnemonic = "BLEZ";
			break;
		case 0x07:
			mnemonic = "BGTZ";
			break;
		case 0x08:
			mnemonic = "ADDI";
			break;
		case 0x09:
			mnemonic = "ADDIU";
			break;
		case 0x0A:
			mnemonic = "SLTI";
			break;
		case 0x0B:
			mnemonic = "SLTIU";
			break;
		case 0x0C:
			mnemonic = "ANDI";
			break;
		case 0x0D:
			mnemonic = "ORI";
			break;
		case 0x0E:
			mnemonic = "XORI";
			break;
		case 0x0F:
			mnemonic = "LUI";
			break;
		case 0x10: case 0x11: case 0x12: case 0x13:
			mnemonic = "COPz"; // TODO: coproc
			break;
		case 0x20:
			mnemonic = "LB";
			break;
		case 0x21:
			mnemonic = "LH";
			break;
		case 0x22:
			mnemonic = "LWL";
			break;
		case 0x23:
			mnemonic = "LW";
			break;
		case 0x24:
			mnemonic = "LBU";
			break;
		case 0x25:
			mnemonic = "LHU";
			break;
		case 0x26:
			mnemonic = "LWR";
			break;
		case 0x28:
			mnemonic = "SB";
			break;
		case 0x29:
			mnemonic = "SH";
			break;
		case 0x2A:
			mnemonic = "SWL";
			break;
		case 0x2B:
			mnemonic = "SW";
			break;
		case 0x2E:
			mnemonic = "SWR";
			break;
		case 0x31: case 0x32: case 0x33:
			mnemonic = "LWCz"; // TODO: coproc
			break;
		case 0x39: case 0x3A: case 0x3B:
			mnemonic = "SWCz";
			break;
		default:
			mnemonic = "=?*";
			break;
	}
	printf("%s %s,%s,%04X\n", mnemonic, regs[rt], regs[rs], im);
}
