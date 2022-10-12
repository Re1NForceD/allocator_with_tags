#define TESTING

#include <allocator.hpp>
#include <block.hpp>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <iostream>
#include <list>
#include <string>
#include <tree.hpp>

// EXPECT_EQ(sizeof(coder_t), sizeof(Coder));
// EXPECT_STREQ(buf_hello, coder.buf());
// EXPECT_NE(buf_company, coder.buf());
// EXPECT_THROW(coder.set(0, 1), std::logic_error);
// EXPECT_STRNE(buf_hello, coder.buf());
// ASSERT_EQ(buf[i], *(coder.buf()+i)) << "i is: " << i;

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Lt;

#define SIZE_MAX 0xFFFF
#define INRANGE(s) AllOf(Ge(s), Lt(s + alignof(std::max_align_t)))
// #define NEED_CORRECT

#include <alignDefs.hpp>

inline block* getBlock(const void* mem)
{
    return (block*)((char*)mem - sizeof(struct block));
}

TEST(Allocator, MAX_SIZE_overflow_test)
{
    auto allocBlock = mem_alloc(SIZE_MAX);
    EXPECT_THAT(getBlock(allocBlock)->sizeCurrent, INRANGE(SIZE_MAX));
    mem_show();
    mem_free(allocBlock);
    mem_show();
}

TEST(Allocator, test_first_alloc_free)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = 20;
    const auto b2S = 40;
    const auto b3S = 60;
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b2S);
    auto block3 = mem_alloc(b3S);

    std::cout << "Init state:" << std::endl;
    mem_show();

    mem_free(block1);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1)->sizeCurrent, ROUND_BYTES(b1S));
    EXPECT_FALSE(getBlock(block1)->isBusy());

    EXPECT_EQ(getBlock(block2)->sizePrevious, getBlock(block1)->sizeCurrent);
    EXPECT_EQ(getBlock(block2)->sizeCurrent, ROUND_BYTES(b2S));
    EXPECT_TRUE(getBlock(block2)->isBusy());

    EXPECT_EQ(getBlock(block3)->sizePrevious, getBlock(block2)->sizeCurrent);
    EXPECT_EQ(getBlock(block3)->sizeCurrent, ROUND_BYTES(b3S));
    EXPECT_TRUE(getBlock(block3)->isBusy());

    mem_free(block2);
    mem_free(block3);
    mem_show();
}

TEST(Allocator, test_last_alloc_free)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = 20;
    const auto b2S = 40;
    const auto b3S = 60;
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b2S);
    auto block3 = mem_alloc(b3S);
    mem_show();

    auto block4Size = getBlock(block3)->next()->sizeCurrent;
    mem_free(block3);
    mem_show();

    EXPECT_EQ(getBlock(block1)->sizeCurrent, ROUND_BYTES(b1S));
    EXPECT_TRUE(getBlock(block1)->isBusy());

    EXPECT_EQ(getBlock(block2)->sizePrevious, getBlock(block1)->sizeCurrent);
    EXPECT_EQ(getBlock(block2)->sizeCurrent, ROUND_BYTES(b2S));
    EXPECT_TRUE(getBlock(block2)->isBusy());

    EXPECT_EQ(getBlock(block3)->sizePrevious, getBlock(block2)->sizeCurrent);
    EXPECT_EQ(getBlock(block3)->sizeCurrent, ROUND_BYTES(b3S) + sizeof(struct block) + block4Size);
    EXPECT_FALSE(getBlock(block3)->isBusy());

    mem_free(block1);
    mem_free(block2);
    mem_show();
}

TEST(Allocator, test_busy_curr_busy)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = 20;
    const auto b2S = 40;
    const auto b3S = 60;
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b2S);
    auto block3 = mem_alloc(b3S);

    std::cout << "Init state:" << std::endl;
    mem_show();

    mem_free(block2);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1)->sizeCurrent, ROUND_BYTES(b1S));
    EXPECT_TRUE(getBlock(block1)->isBusy());

    EXPECT_EQ(getBlock(block2)->sizePrevious, getBlock(block1)->sizeCurrent);
    EXPECT_EQ(getBlock(block2)->sizeCurrent, ROUND_BYTES(b2S));
    EXPECT_FALSE(getBlock(block2)->isBusy());

    EXPECT_EQ(getBlock(block3)->sizePrevious, getBlock(block2)->sizeCurrent);
    EXPECT_EQ(getBlock(block3)->sizeCurrent, ROUND_BYTES(b3S));
    EXPECT_TRUE(getBlock(block3)->isBusy());

    mem_free(block1);
    mem_free(block3);
    mem_show();
}

TEST(Allocator, test_free_curr_busy)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = 20;
    const auto b2S = 40;
    const auto b3S = 60;
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b2S);
    auto block3 = mem_alloc(b3S);
    mem_free(block1);

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto block2Size = getBlock(block2)->sizeCurrent;
    mem_free(block2);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1)->sizeCurrent, ROUND_BYTES(b1S) + sizeof(struct block) + block2Size);
    EXPECT_FALSE(getBlock(block1)->isBusy());

    EXPECT_EQ(getBlock(block3)->sizePrevious, getBlock(block1)->sizeCurrent);
    EXPECT_EQ(getBlock(block3)->sizeCurrent, ROUND_BYTES(b3S));
    EXPECT_TRUE(getBlock(block3)->isBusy());

    mem_free(block3);
    mem_show();
}

TEST(Allocator, test_busy_curr_free)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = 20;
    const auto b2S = 40;
    const auto b3S = 60;
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b2S);

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto block3Size = getBlock(block2)->next()->sizeCurrent;
    mem_free(block2);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1)->sizeCurrent, ROUND_BYTES(b1S));
    EXPECT_TRUE(getBlock(block1)->isBusy());

    EXPECT_EQ(getBlock(block2)->sizePrevious, ROUND_BYTES(b1S));
    EXPECT_EQ(getBlock(block2)->sizeCurrent, ROUND_BYTES(b2S) + sizeof(struct block) + block3Size);
    EXPECT_FALSE(getBlock(block2)->isBusy());

    mem_free(block1);
    mem_show();
}

TEST(Allocator, test_free_curr_free)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = 20;
    const auto b2S = 40;
    const auto b3S = 60;
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b2S);
    mem_free(block1);

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto block3Size = getBlock(block2)->next()->sizeCurrent;
    mem_free(block2);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1)->sizeCurrent, ROUND_BYTES(b1S) + sizeof(struct block) + ROUND_BYTES(b2S) + sizeof(struct block) + block3Size);
    EXPECT_FALSE(getBlock(block1)->isBusy());
}

TEST(Allocator, test_realloc_in_place_decrease_size)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = 80;
    const auto b1S_new = 20;
    auto block1 = mem_alloc(b1S);

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto block2 = mem_realloc(block1, b1S_new);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1), getBlock(block2));
    EXPECT_EQ(getBlock(block1)->sizeCurrent, ROUND_BYTES(b1S_new));
    EXPECT_TRUE(getBlock(block1)->isBusy());

    mem_free(block1);
    mem_show();
}

TEST(Allocator, test_realloc_in_place_increase_size)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = 40;
    const auto b1S_new = 60;
    auto block1 = mem_alloc(b1S);

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto block2 = mem_realloc(block1, b1S_new);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1), getBlock(block2));
    EXPECT_EQ(getBlock(block1)->sizeCurrent, ROUND_BYTES(b1S_new));
    EXPECT_TRUE(getBlock(block1)->isBusy());

    mem_free(block1);
    mem_show();
}

TEST(Allocator, test_realloc_new_place_increase_size)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = 40;
    const auto b1S_new = 60;
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b1S);

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto block3 = mem_realloc(block1, b1S_new);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_NE(getBlock(block1), getBlock(block3));
    EXPECT_FALSE(getBlock(block1)->isBusy());
    EXPECT_TRUE(getBlock(block3)->isBusy());
    EXPECT_EQ(getBlock(block3)->sizeCurrent, ROUND_BYTES(b1S_new));

    mem_free(block2);
    mem_free(block3);
    mem_show();
}

TEST(Allocator, test_realloc_mix)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = ROUND_BYTES(50);
    const auto b1S_b = ROUND_BYTES(60);
    const auto b1S_s = ROUND_BYTES(40);
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b1S);
    auto block3 = mem_alloc(b1S);
    auto block4 = mem_alloc(b1S);

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto block5 = mem_realloc(block1, b1S_b);
    auto block6 = mem_realloc(block2, b1S_s);
    auto block7 = mem_realloc(block3, b1S_b);
    auto block8 = mem_realloc(block4, b1S_s);

    std::cout << "End state:" << std::endl;
    mem_show();

    // std::cout << "Block1: " << getBlock(block1) << " Block5: " << getBlock(block5) << std::endl;
    EXPECT_NE(getBlock(block1), getBlock(block5));
    EXPECT_EQ(getBlock(block1)->sizeCurrent, b1S);
    EXPECT_FALSE(getBlock(block1)->isBusy());
    EXPECT_EQ(getBlock(block5)->sizeCurrent, b1S_b);
    EXPECT_TRUE(getBlock(block5)->isBusy());

    // std::cout << "Block2: " << getBlock(block2) << " Block6: " << getBlock(block6) << std::endl;
    EXPECT_EQ(getBlock(block2), getBlock(block6));
    EXPECT_EQ(getBlock(block2)->sizeCurrent, b1S - b1S_s >= sizeof(struct block) + sizeof(struct Node) ? b1S_s : b1S);
    EXPECT_TRUE(getBlock(block2)->isBusy());

    // std::cout << "Block3: " << getBlock(block3) << " Block7: " << getBlock(block7) << std::endl;
    EXPECT_NE(getBlock(block3), getBlock(block7));
    EXPECT_EQ(getBlock(block3)->sizeCurrent, b1S);
    EXPECT_FALSE(getBlock(block3)->isBusy());
    EXPECT_EQ(getBlock(block7)->sizeCurrent, b1S_b);
    EXPECT_TRUE(getBlock(block7)->isBusy());

    // std::cout << "Block4: " << getBlock(block4) << " Block8: " << getBlock(block8) << std::endl;
    EXPECT_EQ(getBlock(block4), getBlock(block8));
    EXPECT_EQ(getBlock(block4)->sizeCurrent, b1S - b1S_s >= sizeof(struct block) + sizeof(struct Node) ? b1S_s : b1S);
    EXPECT_TRUE(getBlock(block4)->isBusy());

    mem_free(block5);
    mem_free(block6);
    mem_free(block7);
    mem_free(block8);
    mem_show();
}

TEST(Allocator, test_alv_tree)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = 40;
    const auto b2S = 20;
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b1S);
    auto block3 = mem_alloc(b1S);
    auto block4 = mem_alloc(b1S);
    auto block5 = mem_alloc(b2S);
    auto block6 = mem_alloc(b2S);
    auto block7 = mem_alloc(b2S);
    auto block8 = mem_alloc(b2S);

    std::cout << "Init state:" << std::endl;
    mem_show();

    mem_free(block1);
    mem_free(block3);
    mem_free(block5);
    mem_free(block7);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(treeRoot->getKey(), ROUND_BYTES(b1S));
    EXPECT_NE(treeRoot->nextSameKey, nullptr);
    EXPECT_EQ(treeRoot->nextSameKey->getKey(), ROUND_BYTES(b1S));
    EXPECT_EQ(treeRoot->left->getKey(), ROUND_BYTES(b2S));
    EXPECT_NE(treeRoot->left->nextSameKey, nullptr);
    EXPECT_EQ(treeRoot->left->nextSameKey->getKey(), ROUND_BYTES(b2S));
    EXPECT_EQ(treeRoot->right->getKey(), ((block*)block8 - 1)->next()->sizeCurrent);
    EXPECT_EQ(treeRoot->right->nextSameKey, nullptr);

    mem_free(block2);
    mem_free(block4);
    mem_free(block6);
    mem_free(block8);
    mem_show();
}

size_t calcCheckSum(block* b)
{
    if (b->sizeCurrent <= sizeof(size_t)) return 0;
    size_t* val = (size_t*)(b + 1);
    size_t sum = 0;
    while ((char*)val != ((char*)(b + 1) + sizeof(size_t) * 3))
    {
        if ((char*)val == (char*)(b + 1)) {
            val++;
            continue;
        }
        sum += *val;
        val++;
    }
    return sum;
}

void setCheckSum(block* b)
{
    size_t* blockCheckSum = (size_t*)(b + 1);
    *blockCheckSum = calcCheckSum(b);
}

size_t getCheckSum(block* b)
{
    return *(size_t*)(b + 1);
}

void printValues(block* b)
{
    std::cout << getCheckSum(b) << " " << *((size_t*)(b + 1) + 1) << " " << *((size_t*)(b + 1) + 2) << std::endl;
}

bool validateBlock(block* b)
{
    if (getCheckSum(b) == calcCheckSum(b)) return true;
    else return false;
}

#include <ctime>
#include <stdlib.h>

TEST(Allocator, test_auto_tester_check_sum)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    size_t blockAmount = 100;
    const size_t newBlockMinSize = sizeof(struct block) + sizeof(struct Node);
    const size_t bS = 3 * sizeof(size_t) + newBlockMinSize;
    block** allocBlocks = (block**)mem_alloc(blockAmount * sizeof(block*));

    srand(time(nullptr));
    for (size_t i{ 0 }; i < blockAmount; ++i)
    {
        allocBlocks[i] = (block*)mem_alloc(bS) - 1;
        size_t* content = (size_t*)(allocBlocks[i] + 1);
        content[1] = (size_t)(rand() % 10000 + 1);
        content[2] = (size_t)(rand() % 10000 + 1);
        setCheckSum(allocBlocks[i]);
    }

    mem_show();

    for (size_t i{ 0 }; i < blockAmount; ++i)
    {
        std::cout << std::endl << i;
        EXPECT_TRUE(getCheckSum(allocBlocks[i]) == calcCheckSum(allocBlocks[i]));

        auto randV = rand() % 3 + 1;
        if (randV % 3 == 0)
        {
            std::cout << " realloc in-place " << allocBlocks[i] << std::endl;
            auto checkSum = getCheckSum(allocBlocks[i]);
            allocBlocks[i] = (block*)mem_realloc(allocBlocks[i] + 1, bS - newBlockMinSize) - 1;
            ASSERT_EQ(getCheckSum(allocBlocks[i]), checkSum) << i;
        }
        else if (randV % 3 == 1)
        {
            std::cout << " realloc new-place " << allocBlocks[i] << std::endl;
            auto checkSum = getCheckSum(allocBlocks[i]);
            allocBlocks[i] = (block*)mem_realloc(allocBlocks[i] + 1, bS + 16) - 1;
            ASSERT_EQ(getCheckSum(allocBlocks[i]), checkSum) << i;
        }
        else
        {
            std::cout << " free " << allocBlocks[i] << std::endl;
            mem_free(allocBlocks[i] + 1);
            allocBlocks[i] = nullptr;
        }
    }

    mem_show();

    for (size_t i{ 0 }; i < blockAmount; ++i)
    {
        if (!allocBlocks[i]) continue;
        ASSERT_EQ(getCheckSum(allocBlocks[i]), calcCheckSum(allocBlocks[i])) << i;
    }
}
