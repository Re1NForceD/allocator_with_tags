#define TESTING

#include <allocator.hpp>
#include <block.hpp>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <iostream>
#include <list>
#include <string>
#include <tree.hpp>

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Lt;

#define SIZE_MAX 0xFFFF
#define INRANGE(s) AllOf(Ge(s), Lt(s + alignof(std::max_align_t)))
#define FIT_SIZE(s) (ROUND_BYTES(s) < sizeof(struct Node) ? sizeof(struct Node) : ROUND_BYTES(s))
// #define NEED_CORRECT

#include <alignDefs.hpp>

inline block* getBlock(const void* mem)
{
    return (block*)((char*)mem - sizeof(struct block));
}

TEST(Allocator, MAX_SIZE_overflow_test)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    size_t* allocBlock = (size_t*)mem_alloc(SIZE_MAX);
    EXPECT_THAT(getBlock(allocBlock)->getCurrentSize(), INRANGE(SIZE_MAX));
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

    std::cout << "node size: " << sizeof(struct Node) << std::endl;
    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1)->getCurrentSize(), FIT_SIZE(b1S));
    EXPECT_FALSE(getBlock(block1)->isBusy());

    EXPECT_EQ(getBlock(block2)->getPreviousSize(), getBlock(block1)->getCurrentSize());
    EXPECT_EQ(getBlock(block2)->getCurrentSize(), FIT_SIZE(b2S));
    EXPECT_TRUE(getBlock(block2)->isBusy());

    EXPECT_EQ(getBlock(block3)->getPreviousSize(), getBlock(block2)->getCurrentSize());
    EXPECT_EQ(getBlock(block3)->getCurrentSize(), FIT_SIZE(b3S));
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

    auto block4Size = getBlock(block3)->next()->getCurrentSize();
    mem_free(block3);
    mem_show();

    EXPECT_EQ(getBlock(block1)->getCurrentSize(), FIT_SIZE(b1S));
    EXPECT_TRUE(getBlock(block1)->isBusy());

    EXPECT_EQ(getBlock(block2)->getPreviousSize(), getBlock(block1)->getCurrentSize());
    EXPECT_EQ(getBlock(block2)->getCurrentSize(), FIT_SIZE(b2S));
    EXPECT_TRUE(getBlock(block2)->isBusy());

    EXPECT_EQ(getBlock(block3)->getPreviousSize(), getBlock(block2)->getCurrentSize());
    EXPECT_EQ(getBlock(block3)->getCurrentSize(), FIT_SIZE(b3S) + sizeof(struct block) + block4Size);
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

    EXPECT_EQ(getBlock(block1)->getCurrentSize(), FIT_SIZE(b1S));
    EXPECT_TRUE(getBlock(block1)->isBusy());

    EXPECT_EQ(getBlock(block2)->getPreviousSize(), getBlock(block1)->getCurrentSize());
    EXPECT_EQ(getBlock(block2)->getCurrentSize(), FIT_SIZE(b2S));
    EXPECT_FALSE(getBlock(block2)->isBusy());

    EXPECT_EQ(getBlock(block3)->getPreviousSize(), getBlock(block2)->getCurrentSize());
    EXPECT_EQ(getBlock(block3)->getCurrentSize(), FIT_SIZE(b3S));
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

    auto block2Size = getBlock(block2)->getCurrentSize();
    mem_free(block2);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1)->getCurrentSize(), FIT_SIZE(b1S) + sizeof(struct block) + block2Size);
    EXPECT_FALSE(getBlock(block1)->isBusy());

    EXPECT_EQ(getBlock(block3)->getPreviousSize(), getBlock(block1)->getCurrentSize());
    EXPECT_EQ(getBlock(block3)->getCurrentSize(), FIT_SIZE(b3S));
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

    auto block3Size = getBlock(block2)->next()->getCurrentSize();
    mem_free(block2);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1)->getCurrentSize(), FIT_SIZE(b1S));
    EXPECT_TRUE(getBlock(block1)->isBusy());

    EXPECT_EQ(getBlock(block2)->getPreviousSize(), FIT_SIZE(b1S));
    EXPECT_EQ(getBlock(block2)->getCurrentSize(), FIT_SIZE(b2S) + sizeof(struct block) + block3Size);
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

    auto block3Size = getBlock(block2)->next()->getCurrentSize();
    mem_free(block2);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1)->getCurrentSize(), FIT_SIZE(b1S) + sizeof(struct block) + FIT_SIZE(b2S) + sizeof(struct block) + block3Size);
    EXPECT_FALSE(getBlock(block1)->isBusy());
}

TEST(Allocator, test_realloc_in_place_decrease_size)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = 100;
    const auto b1S_new = 20;
    auto block1 = mem_alloc(b1S);

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto block2 = mem_realloc(block1, b1S_new);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1), getBlock(block2));
    EXPECT_EQ(getBlock(block1)->getCurrentSize(), FIT_SIZE(b1S_new));
    EXPECT_TRUE(getBlock(block1)->isBusy());

    mem_free(block2);
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
    EXPECT_EQ(getBlock(block1)->getCurrentSize(), FIT_SIZE(b1S_new));
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
    EXPECT_EQ(getBlock(block3)->getCurrentSize(), FIT_SIZE(b1S_new));

    mem_free(block2);
    mem_free(block3);
    mem_show();
}

TEST(Allocator, test_realloc_mix)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    const auto b1S = FIT_SIZE(100);
    const auto b1S_b = FIT_SIZE(150);
    const auto b1S_s1 = FIT_SIZE(80);
    const auto b1S_s2 = FIT_SIZE(24);
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b1S);
    auto block3 = mem_alloc(b1S);
    auto block4 = mem_alloc(b1S);

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto block6 = mem_realloc(block2, b1S_b);
    auto block5 = mem_realloc(block1, b1S_b);
    auto block7 = mem_realloc(block3, b1S_s1);
    auto block8 = mem_realloc(block4, b1S_s2);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block1), getBlock(block5));
    EXPECT_EQ(getBlock(block5)->getCurrentSize(), b1S_b);
    EXPECT_TRUE(getBlock(block5)->isBusy());
    EXPECT_EQ(getBlock(block5)->prev(), nullptr);
    EXPECT_EQ(getBlock(block5)->getPreviousSize(), 0);

    EXPECT_NE(getBlock(block2), getBlock(block6));
    EXPECT_EQ(getBlock(block6)->getCurrentSize(), b1S_b);
    EXPECT_TRUE(getBlock(block6)->isBusy());
    EXPECT_EQ(getBlock(block6)->prev(), getBlock(block8)->next());
    EXPECT_EQ(getBlock(block6)->getPreviousSize(), b1S - b1S_s2 - sizeof(struct block));

    EXPECT_EQ(getBlock(block3), getBlock(block7));
    EXPECT_EQ(getBlock(block7)->getCurrentSize(), b1S - b1S_s1 >= sizeof(struct block) + sizeof(struct Node) ? b1S_s1 : b1S);
    EXPECT_TRUE(getBlock(block7)->isBusy());
    EXPECT_EQ(getBlock(block7)->prev(), getBlock(block5)->next());

    EXPECT_EQ(getBlock(block4), getBlock(block8));
    EXPECT_EQ(getBlock(block8)->getCurrentSize(), b1S - b1S_s2 >= sizeof(struct block) + sizeof(struct Node) ? b1S_s2 : b1S);
    EXPECT_TRUE(getBlock(block8)->isBusy());

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

    EXPECT_EQ(treeRoot->getKey(), FIT_SIZE(b1S));
    EXPECT_NE(treeRoot->nextSameKey, nullptr);
    EXPECT_EQ(treeRoot->nextSameKey->getKey(), FIT_SIZE(b1S));
    EXPECT_EQ(treeRoot->left->getKey(), FIT_SIZE(b2S));
    EXPECT_NE(treeRoot->left->nextSameKey, nullptr);
    EXPECT_EQ(treeRoot->left->nextSameKey->getKey(), FIT_SIZE(b2S));
    EXPECT_EQ(treeRoot->right->getKey(), ((block*)block8 - 1)->next()->getCurrentSize());
    EXPECT_EQ(treeRoot->right->nextSameKey, nullptr);

    mem_free(block2);
    mem_free(block4);
    mem_free(block6);
    mem_free(block8);
    mem_show();
}

size_t calcCheckSum(size_t* b)
{
    return b[1] + b[2];
}

void setCheckSum(size_t* b)
{
    *b = calcCheckSum(b);
}

size_t getCheckSum(size_t* b)
{
    return *b;
}

void printValues(size_t* b)
{
    std::cout << getCheckSum(b) << " " << b[1] << " " << b[2] << std::endl;
}

bool validateBlock(size_t* b)
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
    size_t** allocBlocks = (size_t**)mem_alloc(blockAmount * sizeof(block*));

    srand(time(nullptr));
    for (size_t i{ 0 }; i < blockAmount; ++i)
    {
        allocBlocks[i] = (size_t*)mem_alloc(bS);
        allocBlocks[i][1] = (size_t)(rand() % 10000 + 1);
        allocBlocks[i][2] = (size_t)(rand() % 10000 + 1);
        setCheckSum(allocBlocks[i]);
    }

    mem_show();

    for (size_t i{ 0 }; i < blockAmount; ++i)
    {
        size_t j = rand() % blockAmount;
        if (!allocBlocks[j]) continue;

        std::cout << j;
        EXPECT_TRUE(getCheckSum(allocBlocks[j]) == calcCheckSum(allocBlocks[j]));

        auto randV = rand() % 3;

        if (randV % 3 == 0)
        {
            std::cout << " realloc in-place " << allocBlocks[j] << std::endl;
            auto checkSum = getCheckSum(allocBlocks[j]);
            allocBlocks[j] = (size_t*)mem_realloc(allocBlocks[j], bS - newBlockMinSize);
            ASSERT_EQ(getCheckSum(allocBlocks[j]), checkSum) << j;
        }
        else if (randV % 3 == 1)
        {
            std::cout << " realloc new-place " << allocBlocks[j] << std::endl;
            auto checkSum = getCheckSum(allocBlocks[j]);
            allocBlocks[j] = (size_t*)mem_realloc(allocBlocks[j], bS + 16);
            ASSERT_EQ(getCheckSum(allocBlocks[j]), checkSum) << j;
        }
        else
        {
            std::cout << " free " << allocBlocks[j] << std::endl;
            mem_free(allocBlocks[j]);
            allocBlocks[j] = nullptr;
        }
    }

    mem_show();

    for (size_t i{ 0 }; i < blockAmount; ++i)
    {
        if (!allocBlocks[i]) continue;
        ASSERT_EQ(getCheckSum(allocBlocks[i]), calcCheckSum(allocBlocks[i])) << i;
        mem_free(allocBlocks[i]);
    }
    mem_free(allocBlocks);

    mem_show();
}

TEST(Allocator, allocate_more_than_default_arena)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    auto littleBlock = mem_alloc(32);
    auto allocBlock = mem_alloc(4096 * 20);
    mem_show();

    EXPECT_EQ(getBlock(allocBlock)->getCurrentSize(), FIT_SIZE(4096 * 21 - sizeof(struct block)));

    mem_free(allocBlock);
    mem_free(littleBlock);
    mem_show();
}

TEST(Allocator, realloc_big_block_less_than)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    auto allocBlock = mem_alloc(4096 * 20);
    auto s1 = getBlock(allocBlock)->getCurrentSize();

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto reallocBlock = mem_realloc(allocBlock, 4096 * 19);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(reallocBlock)->getCurrentSize(), s1);

    mem_free(reallocBlock);
    mem_show();
}

TEST(Allocator, realloc_big_block_more_than)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    auto allocBlock = mem_alloc(4096 * 24);
    auto s1 = FIT_SIZE(4096 * 21 - sizeof(struct block));

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto reallocBlock = mem_realloc(allocBlock, 4096 * 20);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(reallocBlock)->getCurrentSize(), s1);

    mem_free(reallocBlock);
    mem_show();
}

TEST(Allocator, realloc_big_block_to_default_arena)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    auto allocBlock = mem_alloc(4096 * 20);

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto reallocBlock = mem_realloc(allocBlock, 4096 * 10);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(reallocBlock)->getCurrentSize(), FIT_SIZE(4096 * 10));

    mem_free(reallocBlock);
    mem_show();
}

TEST(Allocator, mem_free_inform_kernel)
{
#ifdef NEED_CORRECT
    GTEST_SKIP();
#endif
    size_t* block1 = (size_t*)mem_alloc(4096 * 2);
    block1[0] = 100;
    size_t* block2 = (size_t*)mem_alloc(4096 * 2);

    std::cout << "Init state:" << std::endl;
    mem_show();

    mem_free(block1);
    block2 = (size_t*)mem_realloc(block2, 4096);
    size_t* block3 = (size_t*)mem_alloc(4096);

    std::cout << "End state:" << std::endl;
    mem_show();

    EXPECT_EQ(getBlock(block3)->getCurrentSize(), FIT_SIZE(4096));
    EXPECT_EQ(block3, block1);
    EXPECT_NE(block3[0], 100);
    EXPECT_TRUE(getBlock(block3)->isBusy());

    EXPECT_EQ(getBlock(block2)->getCurrentSize(), FIT_SIZE(4096));
    EXPECT_TRUE(getBlock(block2)->isBusy());

    mem_free(block1);
    mem_free(block2);
    mem_show();
}
