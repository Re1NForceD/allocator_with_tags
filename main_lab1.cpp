// #include <windows.h>
// #include <stdio.h>
#include <allocator.hpp>
#include <iostream>

int main()
{
    int* n = (int*)mem_alloc(sizeof(int));
    // n[0] = 100;
    std::cout << "SHISH!" << std::endl;
    mem_free(n);
	return 0;
}
