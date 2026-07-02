#include <string.h>

#include <stdio.h>
#include "util.h"

extern inline uint64_t get_bits(uint64_t val, uint8_t idx, uint8_t count);
extern inline uint64_t zero_extend16(uint16_t val);
extern inline int64_t zero_extend16_i(int16_t val);
extern inline int64_t sign_extend16_i(int16_t val);
extern inline uint64_t sign_extend16(uint16_t val);
extern inline uint64_t zero_extend32(uint32_t val);
extern inline int64_t zero_extend32_i(int32_t val);
extern inline int64_t sign_extend32_i(int32_t val);
extern inline uint64_t sign_extend32(uint32_t val);
extern inline uint32_t byte_swap_32(uint32_t val);
extern inline uint64_t byte_swap_64(uint64_t val);

minimu_mem_t* minimu_mem_create(uint64_t size) {
	minimu_mem_t* mem = malloc(sizeof(*mem) + (size_t)size);
	if (!mem) {
		fprintf(stderr, "failed to allocate memory\n");
		exit(1);
	}
	mem->size = size;
	return mem;
}

static bool check_range(minimu_mem_t* mem, uint64_t addr, size_t len) {
	return addr <= mem->size && len <= mem->size - addr;
}

static bool check_align(minimu_mem_t* mem, uint64_t addr, size_t len) {
	return len != 0 && addr % len == 0;
}

bool minimu_mem_read(minimu_mem_t* mem, uint64_t addr, void *dst, size_t len) {
	if (!check_range(mem, addr, len)) return false;
	memcpy(dst, mem->data + addr, len);
	return true;
}

bool minimu_mem_write(minimu_mem_t* mem, uint64_t addr, const void *src, size_t len) {
	if (!check_range(mem, addr, len)) return false;
	memcpy(mem->data + addr, src, len);
	return true;
}

uint8_t minimu_mem_read_u8(minimu_mem_t* m, uint64_t addr, bool *ok) {
	uint8_t v = 0;
	bool success = minimu_mem_read(m, addr, &v, 1);
	if (ok) *ok = success;
	return v;
}

void minimu_mem_write_u8(minimu_mem_t* m, uint64_t addr, uint8_t v, bool *ok) {
	bool success = minimu_mem_write(m, addr, &v, 1);
	if (ok) *ok = success;
}
uint16_t minimu_mem_read_u16_le(minimu_mem_t* m, uint64_t addr, bool *ok) {
	if (!check_align(m, addr, 2)) {
		if (ok) *ok = false;
		return 0;
	}
	uint8_t b[2] = {};
	bool success = minimu_mem_read(m, addr, b, 2);
	if (ok) *ok = success;
	return (uint16_t)(b[0] | ((uint16_t)b[1] << 8));
}

uint32_t minimu_mem_read_u32_le(minimu_mem_t* m, uint64_t addr, bool *ok) {
	if (!check_align(m, addr, 4)) {
		if (ok) *ok = false;
		return 0;
	}
	uint8_t b[4] = {};
	bool success = minimu_mem_read(m, addr, b, 4);
	if (ok) *ok = success;
	return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

uint64_t minimu_mem_read_u64_le(minimu_mem_t* m, uint64_t addr, bool *ok) {
	if (!check_align(m, addr, 8)) {
		if (ok) *ok = false;
		return 0;
	}
	uint8_t b[8] = {};
	bool success = minimu_mem_read(m, addr, b, 8);
	if (ok) *ok = success;
	return (uint64_t)b[0] |
		((uint64_t)b[1] << 8) |
		((uint64_t)b[2] << 16) |
		((uint64_t)b[3] << 24) |
		((uint64_t)b[4] << 32) |
		((uint64_t)b[5] << 40) |
		((uint64_t)b[6] << 48) |
		((uint64_t)b[7] << 56);
}

void minimu_mem_write_u16_le(minimu_mem_t* m, uint64_t addr, uint16_t v, bool *ok) {
	if (!check_align(m, addr, 2)) {
		if (ok) *ok = false;
		return;
	}
	uint8_t b[2] = {(uint8_t)(v & 0xff), (uint8_t)(v >> 8)};
	bool success = minimu_mem_write(m, addr, b, 2);
	if (ok) *ok = success;
}

void minimu_mem_write_u32_le(minimu_mem_t* m, uint64_t addr, uint32_t v, bool *ok) {
	if (!check_align(m, addr, 4)) {
		if (ok) *ok = false;
		return;
	}
	uint8_t b[4] = {
		(uint8_t)(v & 0xff),
		(uint8_t)((v >> 8) & 0xff),
		(uint8_t)((v >> 16) & 0xff),
		(uint8_t)((v >> 24) & 0xff),
	};
	bool success = minimu_mem_write(m, addr, b, 4);
	if (ok) *ok = success;
}

void minimu_mem_write_u64_le(minimu_mem_t* m, uint64_t addr, uint64_t v, bool *ok) {
	if (!check_align(m, addr, 8)) {
		if (ok) *ok = false;
		return;
	}
	uint8_t b[8] = {
		(uint8_t)(v & 0xff),
		(uint8_t)((v >> 8) & 0xff),
		(uint8_t)((v >> 16) & 0xff),
		(uint8_t)((v >> 24) & 0xff),
		(uint8_t)((v >> 32) & 0xff),
		(uint8_t)((v >> 40) & 0xff),
		(uint8_t)((v >> 48) & 0xff),
		(uint8_t)((v >> 56) & 0xff),
	};
	bool success = minimu_mem_write(m, addr, b, 8);
	if (ok) *ok = success;
}

uint16_t minimu_mem_read_u16_be(minimu_mem_t* m, uint64_t addr, bool *ok) {
	if (!check_align(m, addr, 2)) {
		if (ok) *ok = false;
		return 0;
	}
	uint8_t b[2] = {};
	bool success = minimu_mem_read(m, addr, b, 2);
	if (ok) *ok = success;
	return (uint16_t)(b[1] | ((uint16_t)b[0] << 8));
}

uint32_t minimu_mem_read_u32_be(minimu_mem_t* m, uint64_t addr, bool *ok) {
	if (!check_align(m, addr, 4)) {
		if (ok) *ok = false;
		return 0;
	}
	uint8_t b[4] = {};
	bool success = minimu_mem_read(m, addr, b, 4);
	if (ok) *ok = success;
	return (uint32_t)b[3] | ((uint32_t)b[2] << 8) | ((uint32_t)b[1] << 16) | ((uint32_t)b[0] << 24);
}

uint64_t minimu_mem_read_u64_be(minimu_mem_t* m, uint64_t addr, bool *ok) {
	if (!check_align(m, addr, 8)) {
		if (ok) *ok = false;
		return 0;
	}
	uint8_t b[8] = {};
	bool success = minimu_mem_read(m, addr, b, 8);
	if (ok) *ok = success;
	return (uint64_t)b[7] |
		((uint64_t)b[6] << 8) |
		((uint64_t)b[5] << 16) |
		((uint64_t)b[4] << 24) |
		((uint64_t)b[3] << 32) |
		((uint64_t)b[2] << 40) |
		((uint64_t)b[1] << 48) |
		((uint64_t)b[0] << 56);
}

void minimu_mem_write_u16_be(minimu_mem_t* m, uint64_t addr, uint16_t v, bool *ok) {
	if (!check_align(m, addr, 2)) {
		if (ok) *ok = false;
		return;
	}
	uint8_t b[2] = {(uint8_t)(v >> 8), (uint8_t)(v & 0xff)};
	bool success = minimu_mem_write(m, addr, b, 2);
	if (ok) *ok = success;
}

void minimu_mem_write_u32_be(minimu_mem_t* m, uint64_t addr, uint32_t v, bool *ok) {
	if (!check_align(m, addr, 4)) {
		if (ok) *ok = false;
		return;
	}
	uint8_t b[4] = {
		(uint8_t)((v >> 24) & 0xff),
		(uint8_t)((v >> 16) & 0xff),
		(uint8_t)((v >> 8) & 0xff),
		(uint8_t)(v & 0xff),
	};
	bool success = minimu_mem_write(m, addr, b, 4);
	if (ok) *ok = success;
}

void minimu_mem_write_u64_be(minimu_mem_t* m, uint64_t addr, uint64_t v, bool *ok) {
	if (!check_align(m, addr, 8)) {
		if (ok) *ok = false;
		return;
	}
	uint8_t b[8] = {
		(uint8_t)((v >> 56) & 0xff),
		(uint8_t)((v >> 48) & 0xff),
		(uint8_t)((v >> 40) & 0xff),
		(uint8_t)((v >> 32) & 0xff),
		(uint8_t)((v >> 24) & 0xff),
		(uint8_t)((v >> 16) & 0xff),
		(uint8_t)((v >> 8) & 0xff),
		(uint8_t)(v & 0xff),
	};
	bool success = minimu_mem_write(m, addr, b, 8);
	if (ok) *ok = success;
}
