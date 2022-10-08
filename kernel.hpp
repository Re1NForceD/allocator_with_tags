#pragma once
#include "block.hpp"

block* kernel_alloc(size_t bytes);
void kernel_free(void* arena);
