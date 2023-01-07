#pragma once

#define DEF_MIN_MAX_CLAMP(T, prefix)                \
    static inline T prefix##min(T x, T y)           \
    {                                               \
        return x < y ? x : y;                       \
    }                                               \
    static inline T prefix##max(T x, T y)           \
    {                                               \
        return x > y ? x : y;                       \
    }                                               \
    static inline T prefix##clamp(T x, T lo, T hi)  \
    {                                               \
        return prefix##min(prefix##max(lo, x), hi); \
    }

DEF_MIN_MAX_CLAMP(int32_t, i32)
DEF_MIN_MAX_CLAMP(uint32_t, u32)
DEF_MIN_MAX_CLAMP(int64_t, i64)
DEF_MIN_MAX_CLAMP(uint64_t, u64)

#undef DEF_MIN_MAX_CLAMP