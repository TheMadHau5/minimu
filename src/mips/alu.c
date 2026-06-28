#include "mips.h"
#include <inttypes.h>
#include <sys/types.h>

static void take_branch(struct minimu_mips *mips, uint64_t pc, int64_t offset_words) {
	mips->has_pending_branch = true;
	mips->pending_branch_target = (uint64_t)((int64_t)pc + 4 + (offset_words << 2));
}

/* funct/opcode helper for R-type instructions that halt the CPU on bad input. */
static void halt(struct minimu_mips *mips, uint8_t status) {
	mips->cpu.status = status;
}

void mips_execute_r(struct minimu_mips *mips, uint32_t instr) {
	uint64_t pc = mips->special[0];
	uint8_t rs = (instr >> 21) & 0x1F;
	uint8_t rt = (instr >> 16) & 0x1F;
	uint8_t rd = (instr >> 11) & 0x1F;
	uint8_t shamt = (instr >> 6) & 0x1F;
	uint8_t funct = instr & 0x3F;
	uint64_t* rsp = &mips->registers[rs];
	uint64_t* rtp = &mips->registers[rt];
	uint64_t* rdp = &mips->registers[rd];
	int64_t rs_s = (int64_t)*rsp;
	int64_t rt_s = (int64_t)*rtp;

	switch (funct) {
		case 0x20: { // ADD
			int32_t a = (int32_t)*rsp, b = (int32_t)*rtp;
			int32_t r;
			if (__builtin_add_overflow(a, b, &r)) {
				halt(mips, MINIMU_STATUS_OVERFLOW);
				return;
			}
			*rdp = (uint64_t)(int64_t)r; // sign-extend 32-bit result
			break;
		}
		case 0x21: // ADDU
			*rdp = (uint64_t)(int64_t)(int32_t)((uint32_t)*rsp + (uint32_t)*rtp);
			break;
		case 0x24: // AND
			*rdp = *rsp & *rtp;
			break;
		case 0xD: // BREAK
			halt(mips, MINIMU_STATUS_BREAKPOINT);
			return;
		case 0x1A: // DIV
			if ((int32_t)*rtp == 0) { halt(mips, MINIMU_STATUS_DIV_BY_ZERO); return; }
			mips->special[1] = (uint64_t)(int64_t)((int32_t)*rsp / (int32_t)*rtp); // LO = quotient
			mips->special[2] = (uint64_t)(int64_t)((int32_t)*rsp % (int32_t)*rtp); // HI = remainder
			break;
		case 0x1B: // DIVU
			if ((uint32_t)*rtp == 0) { halt(mips, MINIMU_STATUS_DIV_BY_ZERO); return; }
			mips->special[1] = (uint64_t)((uint32_t)*rsp / (uint32_t)*rtp);
			mips->special[2] = (uint64_t)((uint32_t)*rsp % (uint32_t)*rtp);
			break;
		case 0x1C: // DMULT
			#if defined(__SIZEOF_INT128__)
			{
				__int128 prod = (__int128)rs_s * (__int128)rt_s;
				mips->special[1] = (uint64_t)prod;
				mips->special[2] = (uint64_t)(prod >> 64);
			}
			#endif
			break;
		case 0x1D: // DMULTU
			#if defined(__SIZEOF_INT128__)
			{
				unsigned __int128 prod = (unsigned __int128)*rsp * (unsigned __int128)*rtp;
				mips->special[1] = (uint64_t)prod;
				mips->special[2] = (uint64_t)(prod >> 64);
			}
			#endif
			break;
		case 0x1E: // DDIV
			if (*rtp == 0) { halt(mips, MINIMU_STATUS_DIV_BY_ZERO); return; }
			mips->special[1] = (uint64_t)(rs_s / rt_s);
			mips->special[2] = (uint64_t)(rs_s % rt_s);
			break;
		case 0x1F: // DDIVU
			if (*rtp == 0) { halt(mips, MINIMU_STATUS_DIV_BY_ZERO); return; }
			mips->special[1] = *rsp / *rtp;
			mips->special[2] = *rsp % *rtp;
			break;
		case 0x2C: // DADD
			{
				int64_t r;
				if (__builtin_add_overflow(rs_s, rt_s, &r)) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
				*rdp = (uint64_t)r;
			}
			break;
		case 0x2D: // DADDU
			*rdp = *rsp + *rtp;
			break;
		case 0x2E: // DSUB
			{
				int64_t r;
				if (__builtin_sub_overflow(rs_s, rt_s, &r)) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
				*rdp = (uint64_t)r;
			}
			break;
		case 0x2F: // DSUBU
			*rdp = *rsp - *rtp;
			break;
		case 0x38: // DSLL
			*rdp = *rtp << shamt;
			break;
		case 0x3A: // DSRL
			*rdp = *rtp >> shamt;
			break;
		case 0x3B: // DSRA
			*rdp = (uint64_t)(rt_s >> shamt);
			break;
		case 0x3C: // DSLL32
			*rdp = *rtp << (shamt + 32);
			break;
		case 0x3E: // DSRL32
			*rdp = *rtp >> (shamt + 32);
			break;
		case 0x3F: // DSRA32
			*rdp = (uint64_t)(rt_s >> (shamt + 32));
			break;
		case 0x14: // DSLLV
			*rdp = *rtp << (*rsp & 0x3F);
			break;
		case 0x16: // DSRLV
			*rdp = *rtp >> (*rsp & 0x3F);
			break;
		case 0x17: // DSRAV
			*rdp = (uint64_t)(rt_s >> (*rsp & 0x3F));
			break;
		case 0x9: // JALR
			*rdp = pc + 8; // link = address after the delay slot
			mips->has_pending_branch = true;
			mips->pending_branch_target = *rsp;
			break;
		case 0x8: // JR
			mips->has_pending_branch = true;
			mips->pending_branch_target = *rsp;
			break;
		case 0x10: // MFHI
			*rdp = mips->special[2];
			break;
		case 0x12: // MFLO
			*rdp = mips->special[1];
			break;
		case 0xB: // MOVN
			if (*rtp != 0) *rdp = *rsp;
			break;
		case 0xA: // MOVZ
			if (*rtp == 0) *rdp = *rsp;
			break;
		case 0x11: // MTHI
			mips->special[2] = *rsp;
			break;
		case 0x13: // MTLO
			mips->special[1] = *rsp;
			break;
		case 0x18: // MULT
			{
				int64_t prod = (int64_t)(int32_t)*rsp * (int64_t)(int32_t)*rtp;
				mips->special[1] = (uint64_t)(int64_t)(int32_t)(prod & 0xFFFFFFFF);
				mips->special[2] = (uint64_t)(int64_t)(int32_t)(prod >> 32);
			}
			break;
		case 0x19: // MULTU
			{
				uint64_t prod = (uint64_t)(uint32_t)*rsp * (uint64_t)(uint32_t)*rtp;
				mips->special[1] = (uint64_t)(int64_t)(int32_t)(uint32_t)(prod & 0xFFFFFFFF);
				mips->special[2] = (uint64_t)(int64_t)(int32_t)(uint32_t)(prod >> 32);
			}
			break;
		case 0x27: // NOR
			*rdp = ~(*rsp | *rtp);
			break;
		case 0x25: // OR
			*rdp = *rsp | *rtp;
			break;
		case 0x0: // SLL
			*rdp = (uint64_t)(int64_t)(int32_t)((uint32_t)*rtp << shamt);
			break;
		case 0x4: // SLLV
			*rdp = (uint64_t)(int64_t)(int32_t)((uint32_t)*rtp << (*rsp & 0x1F));
			break;
		case 0x2A: // SLT
			*rdp = rs_s < rt_s ? 1 : 0;
			break;
		case 0x2B: // SLTU
			*rdp = *rsp < *rtp ? 1 : 0;
			break;
		case 0x3: // SRA
			*rdp = (uint64_t)(int64_t)((int32_t)*rtp >> shamt);
			break;
		case 0x7: // SRAV
			*rdp = (uint64_t)(int64_t)((int32_t)*rtp >> (*rsp & 0x1F));
			break;
		case 0x2: // SRL
			*rdp = (uint64_t)(int64_t)(int32_t)((uint32_t)*rtp >> shamt);
			break;
		case 0x6: // SRLV
			*rdp = (uint64_t)(int64_t)(int32_t)((uint32_t)*rtp >> (*rsp & 0x1F));
			break;
		case 0x22: // SUB
			{
				int32_t a = (int32_t)*rsp, b = (int32_t)*rtp, r;
				if (__builtin_sub_overflow(a, b, &r)) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
				*rdp = (uint64_t)(int64_t)r;
			}
			break;
		case 0x23: // SUBU
			*rdp = (uint64_t)(int64_t)(int32_t)((uint32_t)*rsp - (uint32_t)*rtp);
			break;
		case 0xF: // SYNC
			break;
		case 0xC: // SYSCALL
			mips_execute_syscall(mips, instr);
			break;
		case 0x34: // TEQ
			if (rs_s == rt_s) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		case 0x30: // TGE
			if (rs_s >= rt_s) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		case 0x31: // TGEU
			if (*rsp >= *rtp) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		case 0x32: // TLT
			if (rs_s < rt_s) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		case 0x33: // TLTU
			if (*rsp < *rtp) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		case 0x36: // TNE
			if (rs_s != rt_s) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		case 0x26: // XOR
			*rdp = *rsp ^ *rtp;
			break;
		default:
			halt(mips, MINIMU_STATUS_INVALID_OP);
			return;
	}
	mips->registers[0] = 0;
	if (mips->special[0] == pc) mips->special[0] = pc + 4;
}

void mips_execute_ri(struct minimu_mips *mips, uint32_t instr) {
	uint64_t pc = mips->special[0];
	uint8_t rs = (instr >> 21) & 0x1F;
	uint8_t funct = (instr >> 16) & 0x1F;
	int16_t im = (int16_t)(instr & 0xFFFF);
	uint64_t* rsp = &mips->registers[rs];
	int64_t rs_s = (int64_t)*rsp;
	bool likely = false, taken = false, link = false;

	switch (funct) {
		case 0x1: taken = rs_s >= 0; break;                    // BGEZ
		case 0x11: taken = rs_s >= 0; link = true; break;       // BGEZAL
		case 0x13: taken = rs_s >= 0; link = true; likely = true; break; // BGEZALL
		case 0x3: taken = rs_s >= 0; likely = true; break;      // BGEZL
		case 0x0: taken = rs_s < 0; break;                      // BLTZ
		case 0x10: taken = rs_s < 0; link = true; break;        // BLTZAL
		case 0x12: taken = rs_s < 0; link = true; likely = true; break; // BLTZALL
		case 0x2: taken = rs_s < 0; likely = true; break;       // BLTZL
		default: break;
	}

	switch (funct) {
		case 0x1: case 0x11: case 0x13: case 0x3:
		case 0x0: case 0x10: case 0x12: case 0x2:
			if (link) mips->registers[31] = pc + 8;
			if (taken) {
				take_branch(mips, pc, im);
			} else if (likely) {
				mips->special[0] = pc + 8; // annul delay slot
				return;
			}
			break;
		case 0xC: // TEQI
			if (rs_s == (int64_t)im) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		case 0x8: // TGEI
			if (rs_s >= (int64_t)im) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		case 0x9: // TGEIU
			if (*rsp >= (uint64_t)(int64_t)im) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		case 0xA: // TLTI
			if (rs_s < (int64_t)im) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		case 0xB: // TLTIU
			if (*rsp < (uint64_t)(int64_t)im) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		case 0xE: // TNEI
			if (rs_s != (int64_t)im) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			break;
		default:
			halt(mips, MINIMU_STATUS_INVALID_OP);
			return;
	}
	mips->registers[0] = 0;
	if (mips->special[0] == pc) mips->special[0] = pc + 4;
}

void mips_execute_j(struct minimu_mips *mips, uint32_t instr) {
	uint64_t pc = mips->special[0];
	uint8_t opcode = instr >> 26;
	uint64_t target = (pc & ~(uint64_t)0x0FFFFFFF) | ((uint64_t)(instr & 0x03FFFFFF) << 2);
	mips->has_pending_branch = true;
	mips->pending_branch_target = target;
	if (opcode & 1) {
		// JAL: save the address after the delay slot to $ra
		mips->registers[31] = pc + 8;
	}
	mips->special[0] = pc + 4; // let the delay slot execute next
}

void mips_execute(struct minimu_mips* mips_cpu) {
	struct minimu_mips* mips = mips_cpu;
	uint64_t pc = mips->special[0];

	bool ok = true;
	uint32_t instr = mips_get_memword(mips, pc, &ok);
	if (!ok) {
		mips->cpu.status = MINIMU_STATUS_OOB;
		return;
	}

	if (mips->cpu.flags & 0x2) { // verbose
		int split[6];
		split[0] = instr >> 26;
		split[1] = (instr >> 21) & 0x1F;
		split[2] = (instr >> 16) & 0x1F;
		split[3] = (instr >> 11) & 0x1F;
		split[4] = (instr >> 6) & 0x1F;
		split[5] = instr & 0x3F;
		fprintf(stdout, "%08" PRIX64 ": %06b %05b %05b %05b %05b %06b\n", pc, split[0], split[1], split[2], split[3], split[4], split[5]);
		mips_disasm(instr);
	}

	uint8_t opcode = instr >> 26;
	mips->registers[0] = 0; // reset $zero before executing
	bool apply_after = mips->has_pending_branch;

	switch (opcode) {
		case 0x0:
			mips_execute_r(mips, instr);
			if (apply_after) mips_apply_pending_branch(mips);
			return;
		case 0x1:
			mips_execute_ri(mips, instr);
			if (apply_after) mips_apply_pending_branch(mips);
			return;
		case 0x2: case 0x3:
			mips_execute_j(mips, instr);
			if (apply_after) mips_apply_pending_branch(mips);
			return;
		case 0x11: // COP1
			minimu_mips_fpu_execute(mips, instr);
			if (mips->special[0] == pc) mips->special[0] = pc + 4;
			if (apply_after) mips_apply_pending_branch(mips);
			return;
		case 0x10: // COP0
		case 0x12: // COP2
			// TODO: implement coprocessors 0 and 2
			if (mips->special[0] == pc) mips->special[0] = pc + 4;
			if (apply_after) mips_apply_pending_branch(mips);
			return;
	}

	uint8_t rs = (instr >> 21) & 0x1F;
	uint8_t rt = (instr >> 16) & 0x1F;
	int16_t im = (int16_t)(instr & 0xFFFF);
	uint64_t* rsp = &mips->registers[rs];
	uint64_t* rtp = &mips->registers[rt];
	int64_t rs_s = (int64_t)*rsp;
	uint64_t addr = (uint64_t)((int64_t)*rsp + (int64_t)im);
	bool memok = true;

	switch (opcode) {
		case 0x8: { // ADDI
			int32_t a = (int32_t)*rsp, r;
			if (__builtin_add_overflow(a, (int32_t)im, &r)) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			*rtp = (uint64_t)(int64_t)r;
			break;
		}
		case 0x9: // ADDIU
			*rtp = (uint64_t)(int64_t)(int32_t)((uint32_t)*rsp + (uint32_t)(int32_t)im);
			break;
		case 0x18: { // DADDI
			int64_t r;
			if (__builtin_add_overflow(rs_s, (int64_t)im, &r)) { halt(mips, MINIMU_STATUS_OVERFLOW); return; }
			*rtp = (uint64_t)r;
			break;
		}
		case 0x19: // DADDIU
			*rtp = *rsp + (uint64_t)(int64_t)im;
			break;
		case 0xC: // ANDI
			*rtp = *rsp & (uint64_t)(uint16_t)im;
			break;
		case 0x4: // BEQ
			if (*rtp == *rsp) take_branch(mips, pc, im);
			break;
		case 0x14: // BEQL
			if (*rtp == *rsp) take_branch(mips, pc, im);
			else { mips->special[0] = pc + 8; if (apply_after) mips_apply_pending_branch(mips); return; }
			break;
		case 0x7: // BGTZ
			if (rs_s > 0) take_branch(mips, pc, im);
			break;
		case 0x17: // BGTZL
			if (rs_s > 0) take_branch(mips, pc, im);
			else { mips->special[0] = pc + 8; if (apply_after) mips_apply_pending_branch(mips); return; }
			break;
		case 0x6: // BLEZ
			if (rs_s <= 0) take_branch(mips, pc, im);
			break;
		case 0x16: // BLEZL
			if (rs_s <= 0) take_branch(mips, pc, im);
			else { mips->special[0] = pc + 8; if (apply_after) mips_apply_pending_branch(mips); return; }
			break;
		case 0x5: // BNE
			if (*rtp != *rsp) take_branch(mips, pc, im);
			break;
		case 0x15: // BNEL
			if (*rtp != *rsp) take_branch(mips, pc, im);
			else { mips->special[0] = pc + 8; if (apply_after) mips_apply_pending_branch(mips); return; }
			break;
		case 0x20: { // LB
			uint32_t w = mips_get_memword(mips, addr & ~(uint64_t)3, &memok);
			if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
			int shift = (int)(addr & 3) * 8;
			int8_t b = (int8_t)((w >> shift) & 0xFF);
			*rtp = (uint64_t)(int64_t)b;
			break;
		}
		case 0x24: { // LBU
			uint32_t w = mips_get_memword(mips, addr & ~(uint64_t)3, &memok);
			if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
			int shift = (int)(addr & 3) * 8;
			*rtp = (uint64_t)((w >> shift) & 0xFF);
			break;
		}
		case 0x21: { // LH
			if (addr & 1) { halt(mips, MINIMU_STATUS_OOB); return; }
			uint32_t w = mips_get_memword(mips, addr & ~(uint64_t)3, &memok);
			if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
			int shift = (int)(addr & 2) * 8;
			int16_t h = (int16_t)((w >> shift) & 0xFFFF);
			*rtp = (uint64_t)(int64_t)h;
			break;
		}
		case 0x25: { // LHU
			if (addr & 1) { halt(mips, MINIMU_STATUS_OOB); return; }
			uint32_t w = mips_get_memword(mips, addr & ~(uint64_t)3, &memok);
			if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
			int shift = (int)(addr & 2) * 8;
			*rtp = (uint64_t)((w >> shift) & 0xFFFF);
			break;
		}
		case 0x30: // LL
			if (addr & 3) { halt(mips, MINIMU_STATUS_OOB); return; }
			*rtp = (uint64_t)(int64_t)(int32_t)mips_get_memword(mips, addr, &memok);
			if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
			break;
		case 0xF: // LUI
			*rtp = (uint64_t)(int64_t)(int32_t)((uint32_t)im << 16);
			break;
		case 0x23: // LW
			if (addr & 3) { halt(mips, MINIMU_STATUS_OOB); return; }
			*rtp = (uint64_t)(int64_t)(int32_t)mips_get_memword(mips, addr, &memok);
			if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
			break;
		case 0x27: // LWU
			if (addr & 3) { halt(mips, MINIMU_STATUS_OOB); return; }
			*rtp = (uint64_t)mips_get_memword(mips, addr, &memok);
			if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
			break;
		case 0x37: // LD
			if (addr & 7) { halt(mips, MINIMU_STATUS_OOB); return; }
			*rtp = mips_get_memdword(mips, addr, &memok);
			if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
			break;
		case 0xD: // ORI
			*rtp = *rsp | (uint64_t)(uint16_t)im;
			break;
		case 0x33: // PREF
			break;
		case 0x2F: // CACHE
			break;
		case 0x28: // SB
			{
				uint32_t w = mips_get_memword(mips, addr & ~(uint64_t)3, &memok);
				if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
				int shift = (int)(addr & 3) * 8;
				w = (w & ~(0xFFu << shift)) | (((uint32_t)*rtp & 0xFF) << shift);
				mips_set_memword(mips, addr & ~(uint64_t)3, w, &memok);
			}
			break;
		case 0x38: // SC
			if (addr & 3) { halt(mips, MINIMU_STATUS_OOB); return; }
			mips_set_memword(mips, addr, (uint32_t)*rtp, &memok);
			if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
			*rtp = 1; // success
			break;
		case 0x29: // SH
			{
				if (addr & 1) { halt(mips, MINIMU_STATUS_OOB); return; }
				uint32_t w = mips_get_memword(mips, addr & ~(uint64_t)3, &memok);
				if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
				int shift = (int)(addr & 2) * 8;
				w = (w & ~(0xFFFFu << shift)) | (((uint32_t)*rtp & 0xFFFF) << shift);
				mips_set_memword(mips, addr & ~(uint64_t)3, w, &memok);
			}
			break;
		case 0xA: { // SLTI
			*rtp = rs_s < (int64_t)im ? 1 : 0;
			break;
		}
		case 0xB: { // SLTIU
			*rtp = *rsp < (uint64_t)(int64_t)im ? 1 : 0;
			break;
		}
		case 0x2B: // SW
			if (addr & 3) { halt(mips, MINIMU_STATUS_OOB); return; }
			mips_set_memword(mips, addr, (uint32_t)*rtp, &memok);
			if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
			break;
		case 0x3F: // SD
			if (addr & 7) { halt(mips, MINIMU_STATUS_OOB); return; }
			mips_set_memdword(mips, addr, *rtp, &memok);
			if (!memok) { halt(mips, MINIMU_STATUS_OOB); return; }
			break;
		case 0x22: case 0x26: case 0x2A: case 0x2E: case 0x2C: case 0x2D: case 0x1A: case 0x1B:
			// 0x22: LWL / 0x26: LWR / 0x2A: SWL / 0x2E: SWR / 0x2C: SDL / 0x2D: SDR / 0x1A: LDL / 0x1B: LDR
			// TODO: unaligned load/store
			halt(mips, MINIMU_STATUS_INVALID_OP);
			return;
		case 0xE: // XORI
			*rtp = *rsp ^ (uint64_t)(uint16_t)im;
			break;
		case 0x31: case 0x35: case 0x39: case 0x3D: // LWC1/LDC1/SWC1/SDC1
			// TODO: FPU load/store
			halt(mips, MINIMU_STATUS_INVALID_OP);
			return;
		case 0x32: case 0x3A: // LWC2/SWC2: cop2 doesnt exist
			halt(mips, MINIMU_STATUS_INVALID_OP);
			return;
		default:
			halt(mips, MINIMU_STATUS_INVALID_OP);
			return;
	}
	mips->registers[0] = 0;
	if (mips->special[0] == pc) mips->special[0] = pc + 4;
	if (apply_after) mips_apply_pending_branch(mips);
}
