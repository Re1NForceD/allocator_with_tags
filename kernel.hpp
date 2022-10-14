#pragma once
#include "block.hpp"

extern size_t pageSize;
block* kernel_alloc(size_t bytes, bool default = true);
void kernel_free(void* arena);
