#pragma once

#define ALIGN alignof(std::max_align_t)
#define ROUND_BYTES(s) (((s) + (ALIGN - 1)) & ~(ALIGN - 1))
#define ROUND_BYTES_GEN(s, a) (((s) + (a - 1)) & ~(a - 1))
