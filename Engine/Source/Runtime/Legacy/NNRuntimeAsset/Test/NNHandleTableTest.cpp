/**
 * @file NNHandleTableTest.cpp
 * @brief NNHandleTable 单元测试（Phase 8）。
 */

#include <gtest/gtest.h>

#include "NNAssetHandle.h"

using namespace NN::Runtime::Asset;

TEST(NNHandleTableTest, AllocateAndResolve)
{
	NNHandleTable table;
	int dummy = 42;
	auto handle = table.Allocate(&dummy, 1);

	ASSERT_NE(handle, 0u);
	auto* resolved = table.Resolve(handle);
	ASSERT_NE(resolved, nullptr);
	EXPECT_EQ(resolved, &dummy);
}

TEST(NNHandleTableTest, FreeInvalidatesHandle)
{
	NNHandleTable table;
	int dummy = 42;
	auto handle = table.Allocate(&dummy, 1);
	ASSERT_NE(table.Resolve(handle), nullptr);

	table.Free(handle);
	EXPECT_EQ(table.Resolve(handle), nullptr);
}

TEST(NNHandleTableTest, GenerationAntiAba)
{
	NNHandleTable table;
	int a = 10, b = 20;

	auto handle1 = table.Allocate(&a, 1);
	auto idx1 = static_cast<std::uint32_t>(handle1 & 0xFFFFFFFFull);

	table.Free(handle1);

	/* 重新分配同一索引 slot */
	auto handle2 = table.Allocate(&b, 1);
	auto idx2 = static_cast<std::uint32_t>(handle2 & 0xFFFFFFFFull);
	EXPECT_EQ(idx1, idx2) << "free list 应复用同一索引";

	/* 旧 handle 应失效 */
	EXPECT_EQ(table.Resolve(handle1), nullptr)
		<< "旧 Handle 因 generation 不同应失效";

	/* 新 handle 应有效 */
	EXPECT_EQ(table.Resolve(handle2), &b);
}

TEST(NNHandleTableTest, RefCounts)
{
	NNHandleTable table;
	int dummy = 42;
	auto handle = table.Allocate(&dummy, 1);

	EXPECT_EQ(table.GetRefCount(handle), 0u);

	table.AddRef(handle);
	EXPECT_EQ(table.GetRefCount(handle), 1u);

	table.AddRef(handle);
	table.AddRef(handle);
	EXPECT_EQ(table.GetRefCount(handle), 3u);

	bool zero = table.Release(handle);
	EXPECT_FALSE(zero);
	EXPECT_EQ(table.GetRefCount(handle), 2u);

	zero = table.Release(handle);
	EXPECT_FALSE(zero);
	zero = table.Release(handle);
	EXPECT_TRUE(zero);
	EXPECT_EQ(table.GetRefCount(handle), 0u);
}

TEST(NNHandleTableTest, AllocFreeAlloc_Count)
{
	NNHandleTable table;
	EXPECT_EQ(table.GetAllocatedCount(), 0u);

	int a = 1, b = 2, c = 3;
	auto h1 = table.Allocate(&a, 1);
	auto h2 = table.Allocate(&b, 1);
	EXPECT_EQ(table.GetAllocatedCount(), 2u);

	table.Free(h1);
	EXPECT_EQ(table.GetAllocatedCount(), 1u);

	auto h3 = table.Allocate(&c, 1);
	EXPECT_EQ(table.GetAllocatedCount(), 2u);
}

TEST(NNHandleTableTest, TypeIdStorage)
{
	NNHandleTable table;
	int dummy = 42;
	auto handle = table.Allocate(&dummy, 42);

	EXPECT_EQ(table.GetTypeId(handle), 42u);
}
