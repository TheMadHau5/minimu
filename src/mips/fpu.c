#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "mips.h"
#include "../util.h"

#define FMT_S 0x10u
#define FMT_D 0x11u
#define FMT_W 0x14u
#define FMT_L 0x15u

static inline float fpu_get_f32(const struct minimu_mips *mips, uint8_t reg) {
	uint32_t bits = (uint32_t)(mips->fpu_regs[reg] & 0xffffffffu);
	float v;
	memcpy(&v, &bits, sizeof(v));
	return v;
}

static inline double fpu_get_f64(const struct minimu_mips *mips, uint8_t reg) {
	uint64_t bits = mips->fpu_regs[reg];
	double v;
	memcpy(&v, &bits, sizeof(v));
	return v;
}

static inline int32_t fpu_get_i32(const struct minimu_mips *mips, uint8_t reg) {
	return (int32_t)(mips->fpu_regs[reg] & 0xffffffffu);
}

static inline int64_t fpu_get_i64(const struct minimu_mips *mips, uint8_t reg) {
	return (int64_t)mips->fpu_regs[reg];
}

static inline void fpu_set_f32(struct minimu_mips *mips, uint8_t reg, float v) {
	uint32_t bits;
	memcpy(&bits, &v, sizeof(bits));
	mips->fpu_regs[reg] = (uint64_t)bits;
}

static inline void fpu_set_f64(struct minimu_mips *mips, uint8_t reg, double v) {
	uint64_t bits;
	memcpy(&bits, &v, sizeof(bits));
	mips->fpu_regs[reg] = bits;
}

static inline void fpu_set_i32(struct minimu_mips *mips, uint8_t reg, int32_t v) {
	mips->fpu_regs[reg] = (uint64_t)(int64_t)v;
}

static inline void fpu_set_i64(struct minimu_mips *mips, uint8_t reg, int64_t v) {
	mips->fpu_regs[reg] = (uint64_t)v;
}

static inline int32_t fpu_round_to_i32(double v) {
	return (int32_t)llrint(v);
}

static inline int64_t fpu_round_to_i64(double v) {
	return (int64_t)llrint(v);
}

void minimu_mips_fpu_execute(struct minimu_mips *mips, uint32_t instr)
{
	uint8_t opcode = get_bits(instr, 26, 6);
	uint8_t fmt    = get_bits(instr, 21, 5);
	uint8_t funct  = get_bits(instr, 0, 6);
	uint8_t fd     = get_bits(instr, 6, 5);
	uint8_t fs     = get_bits(instr, 11, 5);
	uint8_t ft     = get_bits(instr, 16, 5);

	if (opcode != 0x11) {
		// bad opcode
		return;
	}

	switch (fmt) {
	case 0x10: {
		float a = fpu_get_f32(mips, fs);
		float b = fpu_get_f32(mips, ft);
		float r;

		switch (funct) {
		case 0x00: r = a + b; fpu_set_f32(mips, fd, r); return; /* ADD.S */
		case 0x01: r = a - b; fpu_set_f32(mips, fd, r); return; /* SUB.S */
		case 0x02: r = a * b; fpu_set_f32(mips, fd, r); return; /* MUL.S */
		case 0x03:
			// TODO: handle divzero
			r = a / b; fpu_set_f32(mips, fd, r); return; /* DIV.S */
		case 0x04:
			// TODO: handle negsqrt
			r = sqrtf(a); fpu_set_f32(mips, fd, r); return; /* SQRT.S */
		case 0x05: r = fabsf(a); fpu_set_f32(mips, fd, r); return; /* ABS.S */
		case 0x06: fpu_set_f32(mips, fd, a); return;              /* MOV.S */
		case 0x07: fpu_set_f32(mips, fd, -a); return;             /* NEG.S */

		// TODO: handle range checks
		case 0x20: /* CVT.D.S */
			fpu_set_f64(mips, fd, (double)a);
			return;
		case 0x21: /* CVT.W.S */
			fpu_set_i32(mips, fd, fpu_round_to_i32((double)a));
			return;
		case 0x25: /* CVT.L.S */
			fpu_set_i64(mips, fd, fpu_round_to_i64((double)a));
			return;

		case 0x24: /* ROUND.W.S */
			fpu_set_i32(mips, fd, (int32_t)llroundf(a));
			return;
		case 0x26: /* TRUNC.W.S */
			fpu_set_i32(mips, fd, (int32_t)truncf(a));
			return;
		case 0x27: /* CEIL.W.S */
			fpu_set_i32(mips, fd, (int32_t)ceilf(a));
			return;
		case 0x28: /* FLOOR.W.S */
			fpu_set_i32(mips, fd, (int32_t)floorf(a));
			return;

		default:
			return;
		}
	}

	case 0x11: {
		double a = fpu_get_f64(mips, fs);
		double b = fpu_get_f64(mips, ft);
		double r;

		switch (funct) {
		case 0x00: r = a + b; fpu_set_f64(mips, fd, r); return; /* ADD.D */
		case 0x01: r = a - b; fpu_set_f64(mips, fd, r); return; /* SUB.D */
		case 0x02: r = a * b; fpu_set_f64(mips, fd, r); return; /* MUL.D */
		case 0x03:
			// TODO: handle divzero
			r = a / b; fpu_set_f64(mips, fd, r); return; /* DIV.D */
		case 0x04:
			// TODO: handle negsqrt
			r = sqrt(a); fpu_set_f64(mips, fd, r); return; /* SQRT.D */
		case 0x05: r = fabs(a); fpu_set_f64(mips, fd, r); return; /* ABS.D */
		case 0x06: fpu_set_f64(mips, fd, a); return;               /* MOV.D */
		case 0x07: fpu_set_f64(mips, fd, -a); return;              /* NEG.D */

		// TODO: handle range checks
		case 0x20: /* CVT.S.D */
			fpu_set_f32(mips, fd, (float)a);
			return;
		case 0x21: /* CVT.W.D */
			fpu_set_i32(mips, fd, fpu_round_to_i32(a));
			return;
		case 0x25: /* CVT.L.D */
			fpu_set_i64(mips, fd, fpu_round_to_i64(a));
			return;

		case 0x24: /* ROUND.W.D */
			fpu_set_i32(mips, fd, (int32_t)llround(a));
			return;
		case 0x26: /* TRUNC.W.D */
			fpu_set_i32(mips, fd, (int32_t)trunc(a));
			return;
		case 0x27: /* CEIL.W.D */
			fpu_set_i32(mips, fd, (int32_t)ceil(a));
			return;
		case 0x28: /* FLOOR.W.D */
			fpu_set_i32(mips, fd, (int32_t)floor(a));
			return;

		default:
			return;
		}
	}

	case 0x14: {
		int32_t a = fpu_get_i32(mips, fs);

		switch (funct) {
		case 0x20: /* CVT.S.W */
			fpu_set_f32(mips, fd, (float)a);
			return;
		case 0x21: /* CVT.D.W */
			fpu_set_f64(mips, fd, (double)a);
			return;

		case 0x24: /* ROUND.W.W */
		case 0x26: /* TRUNC.W.W */
		case 0x27: /* CEIL.W.W */
		case 0x28: /* FLOOR.W.W */
			fpu_set_i32(mips, fd, a);
			return;

		default:
			return;
		}
	}

	case 0x15: {
		int64_t a = fpu_get_i64(mips, fs);

		switch (funct) {
		case 0x20: /* CVT.S.L */
			fpu_set_f32(mips, fd, (float)a);
			return;
		case 0x21: /* CVT.D.L */
			fpu_set_f64(mips, fd, (double)a);
			return;

		case 0x24: /* ROUND.L.L */
		case 0x26: /* TRUNC.L.L */
		case 0x27: /* CEIL.L.L */
		case 0x28: /* FLOOR.L.L */
			fpu_set_i64(mips, fd, a);
			return;

		default:
			return;
		}
	}

	default:
		return;
	}
}
