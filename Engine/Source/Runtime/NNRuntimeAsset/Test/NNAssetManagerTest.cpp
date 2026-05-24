/**
 * @file NNAssetManagerTest.cpp
 * @brief NNAssetManager + GuidHashMap 单元测试（Phase 8）。
 */

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "GuidHashMap.h"

using namespace NN::Runtime::Asset;

/* ======================== GuidHashMap 测试 ======================== */

TEST(GuidHashMapTest, InsertAndFind)
{
	GuidHashMap<std::string> map;

	map.Insert(1, "one");
	map.Insert(2, "two");
	map.Insert(3, "three");

	auto* v1 = map.Find(1);
	ASSERT_NE(v1, nullptr);
	EXPECT_EQ(*v1, "one");

	auto* v2 = map.Find(2);
	ASSERT_NE(v2, nullptr);
	EXPECT_EQ(*v2, "two");

	auto* v3 = map.Find(3);
	ASSERT_NE(v3, nullptr);
	EXPECT_EQ(*v3, "three");

	EXPECT_EQ(map.Find(999), nullptr);
}

TEST(GuidHashMapTest, InsertOverwritesExisting)
{
	GuidHashMap<int> map;

	map.Insert(1, 10);
	map.Insert(1, 20);

	auto* v = map.Find(1);
	ASSERT_NE(v, nullptr);
	EXPECT_EQ(*v, 20);
	EXPECT_EQ(map.Size(), 1u);
}

TEST(GuidHashMapTest, Erase)
{
	GuidHashMap<int> map;

	map.Insert(1, 10);
	map.Insert(2, 20);
	map.Insert(3, 30);
	EXPECT_EQ(map.Size(), 3u);

	EXPECT_TRUE(map.Erase(2));
	EXPECT_EQ(map.Size(), 2u);
	EXPECT_EQ(map.Find(2), nullptr);

	/* 1 和 3 仍可查找 */
	EXPECT_NE(map.Find(1), nullptr);
	EXPECT_NE(map.Find(3), nullptr);

	/* 重复删除应返回 false */
	EXPECT_FALSE(map.Erase(2));
}

TEST(GuidHashMapTest, Contains)
{
	GuidHashMap<int> map;
	map.Insert(42, 100);

	EXPECT_TRUE(map.Contains(42));
	EXPECT_FALSE(map.Contains(99));
}

TEST(GuidHashMapTest, EmptyAndClear)
{
	GuidHashMap<int> map;
	EXPECT_TRUE(map.Empty());
	EXPECT_EQ(map.Size(), 0u);

	map.Insert(1, 10);
	map.Insert(2, 20);
	EXPECT_FALSE(map.Empty());
	EXPECT_EQ(map.Size(), 2u);

	map.Clear();
	EXPECT_TRUE(map.Empty());
	EXPECT_EQ(map.Size(), 0u);
}

TEST(GuidHashMapTest, Rehash_GrowsAutomatically)
{
	GuidHashMap<int> map;

	/* 插入足够多的条目触发 rehash */
	for (std::uint64_t i = 1; i <= 100; ++i)
		map.Insert(i, static_cast<int>(i * 10));

	EXPECT_EQ(map.Size(), 100u);

	/* 所有条目仍可查找 */
	for (std::uint64_t i = 1; i <= 100; ++i)
	{
		auto* v = map.Find(i);
		ASSERT_NE(v, nullptr) << "Key " << i << " not found after rehash";
		EXPECT_EQ(*v, static_cast<int>(i * 10));
	}
}

TEST(GuidHashMapTest, Iteration)
{
	GuidHashMap<int> map;
	map.Insert(10, 100);
	map.Insert(20, 200);
	map.Insert(30, 300);

	int count = 0;
	int sum = 0;
	for (auto& [key, value] : map)
	{
		++count;
		sum += value;
	}

	EXPECT_EQ(count, 3);
	EXPECT_EQ(sum, 600);
}

TEST(GuidHashMapTest, SkipTombstoneKey)
{
	/* TOMBSTONE_KEY (~0ULL) 不应被插入 */
	GuidHashMap<int> map;
	map.Insert(~static_cast<std::uint64_t>(0), 42);

	EXPECT_EQ(map.Size(), 0u);
	EXPECT_EQ(map.Find(~static_cast<std::uint64_t>(0)), nullptr);
}

TEST(GuidHashMapTest, SkipEmptyKey)
{
	/* EMPTY_KEY (0) 不应被插入 */
	GuidHashMap<int> map;
	map.Insert(0, 42);

	EXPECT_EQ(map.Size(), 0u);
	EXPECT_EQ(map.Find(0), nullptr);
}

TEST(GuidHashMapTest, CopySemantics)
{
	GuidHashMap<std::string> map;
	map.Insert(1, "one");
	map.Insert(2, "two");

	auto copy = map;
	EXPECT_EQ(copy.Size(), 2u);
	EXPECT_NE(copy.Find(1), nullptr);
	EXPECT_EQ(*copy.Find(1), "one");

	/* 修改副本不影响原 map */
	copy.Insert(3, "three");
	EXPECT_EQ(copy.Size(), 3u);
	EXPECT_EQ(map.Size(), 2u);
}

TEST(GuidHashMapTest, MoveSemantics)
{
	GuidHashMap<int> map;
	map.Insert(1, 10);
	map.Insert(2, 20);

	auto moved = std::move(map);
	EXPECT_EQ(moved.Size(), 2u);
	EXPECT_EQ(map.Size(), 0u);
	EXPECT_NE(moved.Find(1), nullptr);
}
