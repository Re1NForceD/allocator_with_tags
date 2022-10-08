#define TESTING

#include <allocator.hpp>
#include <block.hpp>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <iostream>
#include <list>
#include <string>

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
    const auto b1S = 20;
    const auto b2S = 40;
    const auto b3S = 60;
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b2S);
    auto block3 = mem_alloc(b3S);
    mem_show();

    mem_free(block1);
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
    const auto b1S = 20;
    const auto b2S = 40;
    const auto b3S = 60;
    auto block1 = mem_alloc(b1S);
    auto block2 = mem_alloc(b2S);
    auto block3 = mem_alloc(b3S);

    std::cout << "Init state:" << std::endl;
    mem_show();

    auto block4Size = getBlock(block3)->next()->sizeCurrent;
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

    // EXPECT_EQ(getBlock(block1)->sizeCurrent, ROUND_BYTES(b1S) + sizeof(struct block) + ROUND_BYTES(b2S) + sizeof(struct block) + block3Size);
    // EXPECT_FALSE(getBlock(block1)->isBusy());
}
