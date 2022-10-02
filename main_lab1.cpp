#include <allocator.hpp>
#include <iostream>
#include <list>
#include <string>

int main()
{
    // std::list<void*> blocks;
    // allocate
    // for (size_t i{0}; i < 10; ++i)
    // {
    //     blocks.push_front(mem_alloc(sizeof(int) * (i + 1)));
    //     // *blocks.begin() = i + 1;
    //     // if (i == 0)
    //     //     mem_show();
    // }
    // mem_show();
    // free
    // for (void* ptr: blocks)
    // {
    //     mem_free(ptr);
    // }
    // mem_show();

    try
    {
        void* n = mem_alloc(-1);
        mem_show();
        mem_free(n);
    }
    catch (std::string err)
    {
        std::cout << err << std::endl;
    }
    catch (...)
    {
        std::cout << "Something went wrong!" << std::endl;
    }
    mem_show();

	return 0;
}
