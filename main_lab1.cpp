#include <allocator.hpp>
#include <iostream>

int main()
{
    int* n = (int*)mem_alloc(sizeof(int));
    n[0] = 100;
    mem_show();
    mem_free(n);
    mem_show();
	return 0;
}
