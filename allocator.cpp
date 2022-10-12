#include "allocator.hpp"
#include "kernel.hpp"
#include "block.hpp"
#include "tree.hpp"
#include "alignDefs.hpp"

#include <iostream>
#include <list>
#include <string>

struct ArenaList
{
    block* list;
    ArenaList() : list{ kernel_alloc(4096, false) }
    {
        list->sizePrevious = 0;
    };
    ~ArenaList() {
        for (size_t i{ 0 }; i < getCount(); ++i)
            kernel_free(*getListPtr() + i);
        kernel_free(list);
    };
    bool addArena(block* ptr)
    {
        if (list->sizePrevious * sizeof(block*) > list->sizeCurrent - sizeof(block*)) return false;
        block** b = getListPtr() + getCount();
        *b = ptr;
        list->sizePrevious++;
        return true;
    };
    bool removeArena(block* ptr)
    {
        if (!getCount()) return false;
        for (size_t i{ 0 }; i < getCount(); ++i)
        {
            block* b = *getListPtr() + i;
            if (b == ptr)
            {
                if (i != getCount() - 1) memcpy(ptr, ptr + 1, (getCount() - i - 1) * sizeof(block*));
                list->sizePrevious--;
                kernel_free(ptr);
                return true;
            }
        }
        return false;
    };
    inline block** getListPtr()
    {
        return (block**)(list + 1);
    };
    inline size_t getCount()
    {
        return list->sizePrevious;
    };
};

static ArenaList arenas;

block* findFit(size_t size);

void* mem_alloc(size_t size)
{
    std::cout << "Request for " << size << " bytes" << std::endl;
    auto alignSize = ROUND_BYTES(size);
    std::cout << "Align for " << ALIGN << "... Align size is " << alignSize << " bytes" << std::endl;
    if (alignSize < size) throw std::string("Size overflow while aligns!");

    block* header = findFit(alignSize);

    Node* node = newNode(header->split(alignSize));
    treeRoot = insertNode(treeRoot, node);

    header->busy |= BUSY;
    return (void*)(header + 1);
}

Node* bestFit(Node* root, size_t key)
{
    if (!root) return nullptr;
    else if (root->getKey() == key) return root;
    else if (root->getKey() > key) {
        Node* node = bestFit(root->left, key);
        return node ? node : root;
    }
    else { // root->getKey() < key
        Node* node = bestFit(root->right, key);
        return node ? node : nullptr;
    }
}

block* findFit(size_t size)
{
    if (Node* node = bestFit(treeRoot, size))
    {
        treeRoot = deleteNode(treeRoot, node);
        return node->getBlock();
    }
    else
    {
        arenas.addArena(kernel_alloc(ROUND_BYTES(size) + sizeof(struct block) + sizeof(struct Node)));
        return *arenas.getListPtr() + (arenas.getCount() - 1);
    }
}

void mem_free(void* ptr)
{
    block* header = (block*)ptr - 1;
    header->busy &= ~BUSY;
    header = header->merge();
    treeRoot = insertNode(treeRoot, newNode(header));

    if (!header->prev() && !header->isBusy() && !header->next())
    {
        if (arenas.getCount() > 1)
        {
            treeRoot = deleteNode(treeRoot, (Node*)(header + 1));
            arenas.removeArena(header);
        }
    }
}

void* mem_realloc(void* ptr, size_t size)
{
    auto alignSize = ROUND_BYTES(size);
    block* b = (block*)ptr - 1;
    if (b->sizeCurrent == alignSize || alignSize < sizeof(struct Node)) return ptr;

    if (alignSize < b->sizeCurrent) // in-place
    {
        block* fromSplit = b->split(alignSize);
        if (fromSplit)
        {
            treeRoot = insertNode(treeRoot, newNode(fromSplit));
            b->next()->merge();
        }
        return ptr;
    }
    else
    {
        if (!b->next()->isBusy() &&
            (!b->prev() || b->prev()->isBusy()) &&
            (b->next()->sizeCurrent + sizeof(struct block) + b->sizeCurrent >= alignSize))
        {
            block* fromSplit = b->merge()->split(alignSize);
            treeRoot = insertNode(treeRoot, newNode(fromSplit));
            return ptr;
        }
        else
        {
            block* newBlock = findFit(alignSize);
            block* fromSplit = newBlock->split(alignSize);
            newBlock->busy |= BUSY;

            if (fromSplit)
            {
                block* merged = fromSplit->merge();
                treeRoot = insertNode(treeRoot, newNode(merged));
            }

            memcpy(newBlock + 1, ptr, b->sizeCurrent);

            b->busy &= ~BUSY;
            treeRoot = insertNode(treeRoot, newNode(b->merge()));

            return (void*)(newBlock + 1);
        }
    }
}

void mem_show()
{
    if (treeRoot)
    {
        std::cout << "AVL Tree:" << std::endl;
        printTree(treeRoot);
        std::cout << std::endl;
    }
    else std::cout << "AVL Tree is empty" << std::endl;

    for (size_t i{ 0 }; i < arenas.getCount(); ++i)
    {
        size_t arenaSize = 0;
        std::cout << "Arena " << ++i << ":" << std::endl;
        int j = 0;
        block* b = *arenas.getListPtr() + i - 1;
        while(true) {
            std::cout << "\tBlock " << ++j << ": " << b << std::endl;
            std::cout << "\t\tprev size " << b->sizePrevious << std::endl;
            std::cout << "\t\tcurr size " << b->sizeCurrent << std::endl;
            std::cout << "\t\tflags: last->" << b->isLast() << " | first->" << b->isFirst() << " | busy->" << b->isBusy() << std::endl;

            arenaSize += sizeof(struct block) + b->sizeCurrent;
            if (b->busy & LAST) break;
            b = b->next();
        }
        std::cout << "\tTotal bytes: " << arenaSize << std::endl;
    }
    if (!arenas.getCount())
        std::cout << "No allocated memory with this allocator" << std::endl;
}
