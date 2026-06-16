#include <stdint.h>

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
