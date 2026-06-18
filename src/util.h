#ifndef MINIMU_UTIL_H
#define MINIMU_UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
	uint64_t size;
	uint8_t data[];
} minimu_mem_t;

minimu_mem_t* minimu_mem_create(uint64_t size);
bool minimu_mem_read(minimu_mem_t* m, uint64_t addr, void *dst, size_t len);
bool minimu_mem_write(minimu_mem_t* m, uint64_t addr, const void *src, size_t len);
uint8_t minimu_mem_read_u8(minimu_mem_t* m, uint64_t addr, bool *ok);
void minimu_mem_write_u8(minimu_mem_t* m, uint64_t addr, uint8_t v, bool *ok);
uint16_t minimu_mem_read_u16_le(minimu_mem_t* m, uint64_t addr, bool *ok);
uint32_t minimu_mem_read_u32_le(minimu_mem_t* m, uint64_t addr, bool *ok);
uint64_t minimu_mem_read_u64_le(minimu_mem_t* m, uint64_t addr, bool *ok);
void minimu_mem_write_u16_le(minimu_mem_t* m, uint64_t addr, uint16_t v, bool *ok);
void minimu_mem_write_u32_le(minimu_mem_t* m, uint64_t addr, uint32_t v, bool *ok);
void minimu_mem_write_u64_le(minimu_mem_t* m, uint64_t addr, uint64_t v, bool *ok);
uint16_t minimu_mem_read_u16_le(minimu_mem_t* m, uint64_t addr, bool *ok);
uint32_t minimu_mem_read_u32_le(minimu_mem_t* m, uint64_t addr, bool *ok);
uint64_t minimu_mem_read_u64_le(minimu_mem_t* m, uint64_t addr, bool *ok);
void minimu_mem_write_u16_le(minimu_mem_t* m, uint64_t addr, uint16_t v, bool *ok);
void minimu_mem_write_u32_le(minimu_mem_t* m, uint64_t addr, uint32_t v, bool *ok);
void minimu_mem_write_u64_le(minimu_mem_t* m, uint64_t addr, uint64_t v, bool *ok);

inline uint64_t get_bits(uint64_t val, uint8_t idx, uint8_t count) {
	return (val >> idx) & ((1ull << count) - 1);
}

inline uint64_t zero_extend16(uint16_t val) {
	return val;
}

inline int64_t zero_extend16_i(int16_t val) {
	return zero_extend16(val);
}

inline int64_t sign_extend16_i(int16_t val) {
	return val;
}

inline uint64_t sign_extend16(uint16_t val) {
	return sign_extend16_i(val);
}

inline uint64_t zero_extend32(uint32_t val) {
	return val;
}

inline int64_t zero_extend32_i(int32_t val) {
	return zero_extend32(val);
}

inline int64_t sign_extend32_i(int32_t val) {
	return val;
}

inline uint64_t sign_extend32(uint32_t val) {
	return sign_extend32_i(val);
}

inline uint32_t byte_swap_32(uint32_t val) {
#if defined(__GNUC__) || defined(__clang__)
	return __builtin_bswap32(val);
#elif defined(_MSC_VER)
	return _byteswap_ulong(val);
#else
	return (val >> 24)
		| (val >> 16 & 0xFF) << 8
		| (val >> 8 & 0xFF) << 16
		| (val & 0xFF) << 24;
#endif
}

inline uint64_t byte_swap_64(uint64_t val) {
#if defined(__GNUC__) || defined(__clang__)
	return __builtin_bswap64(val);
#elif defined(_MSC_VER)
	return _byteswap_uint64(val);
#else
	return (val >> 56)
		| (val >> 48 & 0xFF) << 8
		| (val >> 40 & 0xFF) << 16
		| (val >> 32 & 0xFF) << 24
		| (val >> 24 & 0xFF) << 32
		| (val >> 16 & 0xFF) << 40
		| (val >> 8 & 0xFF) << 48
		| (val & 0xFF) << 56;
#endif
}

#endif // MINIMU_UTIL_H
