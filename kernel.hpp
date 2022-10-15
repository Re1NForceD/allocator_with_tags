#pragma once
#include "block.hpp"

extern size_t pageSize;
block* kernel_alloc(size_t bytes, bool defaultPage = true);
void kernel_free(void* arena);
void kernel_reset(void* ptr, size_t size);
