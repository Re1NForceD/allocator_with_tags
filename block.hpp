#pragma once

#define BUSY  1
#define FIRST 2
#define LAST  4

struct block
{
    size_t sizeCurrent;
    size_t sizePrevious;
    char busy;
    void merge();
    void split(size_t);
    inline block* next() { return (block*)((char*)this + sizeCurrent + sizeof(struct block)); };
    inline block* prev() { return (block*)((char*)this - sizePrevious - sizeof(struct block)); };
};

// struct arena
// {
//     size_t size;
//     block* firstBlock;
//     block* lastBlock;
// };
