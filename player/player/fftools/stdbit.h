/*
 * stdbit.h stub for MSVC
 * Provides C23 bit manipulation functions using MSVC intrinsics
 */

#ifndef STDBIT_H_STUB
#define STDBIT_H_STUB

#include <stdint.h>

#ifdef _MSC_VER
#include <intrin.h>

static inline int stdc_count_ones(unsigned int x)
{
    return (int)__popcnt(x);
}

static inline int stdc_count_ones_ull(unsigned long long x)
{
    return (int)__popcnt64(x);
}

static inline int stdc_trailing_zeros(unsigned int x)
{
    unsigned long index;
    if (_BitScanForward(&index, x))
        return (int)index;
    return 32;
}

static inline int stdc_trailing_zeros_ull(unsigned long long x)
{
    unsigned long index;
    if (_BitScanForward64(&index, x))
        return (int)index;
    return 64;
}

#else
#include <stdbit.h>
#endif

#endif /* STDBIT_H_STUB */
