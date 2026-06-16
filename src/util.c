#include "util.h"

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
