#ifndef CGRAPH_ENDIAN_H
#define CGRAPH_ENDIAN_H

#include <stdint.h>

#if defined(_MSC_VER)
#include <stdlib.h>
#define bswap16(x) _byteswap_ushort(x)
#define bswap32(x) _byteswap_ulong(x)
#define bswap64(x) _byteswap_uint64(x)
#elif defined(__GNUC__) || defined(__clang__)
#define bswap16(x) __builtin_bswap16(x)
#define bswap32(x) __builtin_bswap32(x)
#define bswap64(x) __builtin_bswap64(x)
#endif

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define CGRAPH_LITTLE_ENDIAN 0
#elif defined(__BIG_ENDIAN__) || defined(_BIG_ENDIAN)
#define CGRAPH_LITTLE_ENDIAN 0
#else
#define CGRAPH_LITTLE_ENDIAN 1
#endif

#if CGRAPH_LITTLE_ENDIAN
static uint16_t hton16(const uint16_t x) { return bswap16(x); }
static uint32_t hton32(const uint32_t x) { return bswap32(x); }
static uint64_t hton64(const uint64_t x) { return bswap64(x); }
#else
static uint16_t hton16(const uint16_t x) { return x; }
static uint32_t hton32(const uint32_t x) { return x; }
static uint64_t hton64(const uint64_t x) { return x; }
#endif

static uint16_t ntoh16(const uint16_t x) { return hton16(x); }
static uint32_t ntoh32(const uint32_t x) { return hton32(x); }
static uint64_t ntoh64(const uint64_t x) { return hton64(x); }

#endif //CGRAPH_ENDIAN_H