#pragma once

/**
 * @file GuidHashMap.h
 * @brief 基于 Robin Hood hashing 的 open-addressing hash map（Phase 8 性能优化）。
 *
 * Key = std::uint64_t（用于 guid.low 等场景），Value = 模板参数 V。
 *
 * 设计原则：
 *   - Open addressing + Robin Hood probing，减少 cache miss
 *   - 负载因子上限 0.875，满时 2x rehash
 *   - 不提供线程安全（由调用方保护）
 *   - Header-only，无 .cpp 依赖
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <utility>

namespace NN::Runtime::Asset
{

/**
 * @brief Robin Hood hash map（uint64_t key）。
 *
 * 使用方法：
 *   GuidHashMap<std::shared_ptr<AssetEntry>> map;
 *   map.Insert(guid.low, entry);
 *   auto* p = map.Find(guid.low);
 *   map.Erase(guid.low);
 *   for (auto& [k, v] : map) { ... }
 */
template <typename V>
class GuidHashMap
{
public:
	using KeyType = std::uint64_t;
	static constexpr KeyType EMPTY_KEY = 0;
	static constexpr KeyType TOMBSTONE_KEY = ~static_cast<KeyType>(0);
	static constexpr float MAX_LOAD_FACTOR = 0.875f;

	struct Slot
	{
		KeyType key{EMPTY_KEY};
		V value{};
		std::uint8_t probeDistance{0};  // 距离理想位置的探测距离
	};

	GuidHashMap() { Rehash(INITIAL_CAPACITY); }
	~GuidHashMap() = default;

	GuidHashMap(const GuidHashMap& other)
		: m_Capacity(other.m_Capacity)
		, m_Size(other.m_Size)
	{
		m_Slots = new Slot[m_Capacity];
		std::copy(other.m_Slots, other.m_Slots + m_Capacity, m_Slots);
	}

	GuidHashMap& operator=(const GuidHashMap& other)
	{
		if (this != &other)
		{
			delete[] m_Slots;
			m_Capacity = other.m_Capacity;
			m_Size = other.m_Size;
			m_Slots = new Slot[m_Capacity];
			std::copy(other.m_Slots, other.m_Slots + m_Capacity, m_Slots);
		}
		return *this;
	}

	GuidHashMap(GuidHashMap&& other) noexcept
		: m_Slots(other.m_Slots)
		, m_Capacity(other.m_Capacity)
		, m_Size(other.m_Size)
	{
		other.m_Slots = nullptr;
		other.m_Capacity = 0;
		other.m_Size = 0;
	}

	GuidHashMap& operator=(GuidHashMap&& other) noexcept
	{
		if (this != &other)
		{
			delete[] m_Slots;
			m_Slots = other.m_Slots;
			m_Capacity = other.m_Capacity;
			m_Size = other.m_Size;
			other.m_Slots = nullptr;
			other.m_Capacity = 0;
			other.m_Size = 0;
		}
		return *this;
	}

	/** @brief 查找键，返回值指针（未找到返回 nullptr）。 */
	V* Find(KeyType key) noexcept
	{
		if (key == EMPTY_KEY || key == TOMBSTONE_KEY) return nullptr;
		std::size_t idx = Hash(key) & (m_Capacity - 1);
		std::uint8_t dist = 0;
		while (true)
		{
			Slot& s = m_Slots[idx];
			if (s.key == EMPTY_KEY) return nullptr;
			if (s.key == key) return &s.value;
			if (s.probeDistance < dist) return nullptr;
			idx = (idx + 1) & (m_Capacity - 1);
			++dist;
		}
	}

	const V* Find(KeyType key) const noexcept
	{
		return const_cast<GuidHashMap*>(this)->Find(key);
	}

	/** @brief 插入或更新键值对。 */
	void Insert(KeyType key, V value)
	{
		if (key == EMPTY_KEY || key == TOMBSTONE_KEY) return;

		/* 检查是否需要扩容 */
		if (static_cast<float>(m_Size + 1) / static_cast<float>(m_Capacity) > MAX_LOAD_FACTOR)
			Rehash(m_Capacity * 2);

		InsertInternal(key, std::move(value));
	}

	/** @brief 移除键。 */
	bool Erase(KeyType key) noexcept
	{
		if (key == EMPTY_KEY || key == TOMBSTONE_KEY) return false;
		std::size_t idx = Hash(key) & (m_Capacity - 1);
		std::uint8_t dist = 0;
		while (true)
		{
			Slot& s = m_Slots[idx];
			if (s.key == EMPTY_KEY) return false;
			if (s.key == key)
			{
				/* Robin Hood backshift: 将后续连续 slot 前移 */
				s.key = TOMBSTONE_KEY;
				s.value = V{};
				s.probeDistance = 0;
				--m_Size;

				std::size_t next = (idx + 1) & (m_Capacity - 1);
				while (m_Slots[next].key != EMPTY_KEY && m_Slots[next].probeDistance > 0)
				{
					m_Slots[idx] = std::move(m_Slots[next]);
					m_Slots[idx].probeDistance -= 1;
					m_Slots[next].key = TOMBSTONE_KEY;
					m_Slots[next].value = V{};
					m_Slots[next].probeDistance = 0;
					idx = next;
					next = (next + 1) & (m_Capacity - 1);
				}
				return true;
			}
			if (s.probeDistance < dist) return false;
			idx = (idx + 1) & (m_Capacity - 1);
			++dist;
		}
	}

	/** @brief 是否包含键。 */
	[[nodiscard]] bool Contains(KeyType key) const noexcept { return Find(key) != nullptr; }

	/** @brief 元素数量。 */
	[[nodiscard]] std::size_t Size() const noexcept { return m_Size; }

	/** @brief 是否为空。 */
	[[nodiscard]] bool Empty() const noexcept { return m_Size == 0; }

	/** @brief 容量。 */
	[[nodiscard]] std::size_t Capacity() const noexcept { return m_Capacity; }

	/** @brief 清空所有条目。 */
	void Clear()
	{
		delete[] m_Slots;
		m_Capacity = INITIAL_CAPACITY;
		m_Size = 0;
		m_Slots = new Slot[m_Capacity];
	}

	/* ======================== 迭代器 ======================== */

	class Iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = std::pair<KeyType, V&>;
		using difference_type = std::ptrdiff_t;

		Iterator(Slot* slots, std::size_t capacity, std::size_t index)
			: m_Slots(slots), m_Capacity(capacity), m_Index(index)
		{
			SkipEmpty();
		}

		std::pair<KeyType, V&> operator*() const
		{
			return {m_Slots[m_Index].key, m_Slots[m_Index].value};
		}

		Iterator& operator++()
		{
			++m_Index;
			SkipEmpty();
			return *this;
		}

		bool operator==(const Iterator& other) const { return m_Index == other.m_Index; }
		bool operator!=(const Iterator& other) const { return m_Index != other.m_Index; }

	private:
		void SkipEmpty()
		{
			while (m_Index < m_Capacity
			       && (m_Slots[m_Index].key == EMPTY_KEY || m_Slots[m_Index].key == TOMBSTONE_KEY))
				++m_Index;
		}

		Slot* m_Slots;
		std::size_t m_Capacity;
		std::size_t m_Index;
	};

	class ConstIterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = std::pair<KeyType, const V&>;
		using difference_type = std::ptrdiff_t;

		ConstIterator(const Slot* slots, std::size_t capacity, std::size_t index)
			: m_Slots(slots), m_Capacity(capacity), m_Index(index)
		{
			SkipEmpty();
		}

		std::pair<KeyType, const V&> operator*() const
		{
			return {m_Slots[m_Index].key, m_Slots[m_Index].value};
		}

		ConstIterator& operator++()
		{
			++m_Index;
			SkipEmpty();
			return *this;
		}

		bool operator==(const ConstIterator& other) const { return m_Index == other.m_Index; }
		bool operator!=(const ConstIterator& other) const { return m_Index != other.m_Index; }

	private:
		void SkipEmpty()
		{
			while (m_Index < m_Capacity
			       && (m_Slots[m_Index].key == EMPTY_KEY || m_Slots[m_Index].key == TOMBSTONE_KEY))
				++m_Index;
		}

		const Slot* m_Slots;
		std::size_t m_Capacity;
		std::size_t m_Index;
	};

	Iterator begin() { return Iterator(m_Slots, m_Capacity, 0); }
	Iterator end() { return Iterator(m_Slots, m_Capacity, m_Capacity); }
	ConstIterator begin() const { return ConstIterator(m_Slots, m_Capacity, 0); }
	ConstIterator end() const { return ConstIterator(m_Slots, m_Capacity, m_Capacity); }

private:
	static constexpr std::size_t INITIAL_CAPACITY = 16;

	static std::size_t Hash(KeyType key) noexcept
	{
		/* SplitMix64 混合函数 */
		std::uint64_t x = key;
		x ^= x >> 30;
		x *= 0xbf58476d1ce4e5b9ULL;
		x ^= x >> 27;
		x *= 0x94d049bb133111ebULL;
		x ^= x >> 31;
		return static_cast<std::size_t>(x);
	}

	void InsertInternal(KeyType key, V value)
	{
		std::size_t idx = Hash(key) & (m_Capacity - 1);
		std::uint8_t dist = 0;
		Slot pending{key, std::move(value), dist};

		while (true)
		{
			Slot& s = m_Slots[idx];
			if (s.key == EMPTY_KEY || s.key == TOMBSTONE_KEY)
			{
				s = std::move(pending);
				++m_Size;
				return;
			}
			if (s.key == key)
			{
				s.value = std::move(pending.value);
				return;
			}
			/* Robin Hood: 如果当前位置的条目距离更近，交换 */
			if (s.probeDistance < dist)
			{
				std::swap(s, pending);
			}
			idx = (idx + 1) & (m_Capacity - 1);
			++dist;
			pending.probeDistance = dist;
		}
	}

	void Rehash(std::size_t newCapacity)
	{
		Slot* oldSlots = m_Slots;
		std::size_t oldCapacity = m_Capacity;

		m_Slots = new Slot[newCapacity];
		m_Capacity = newCapacity;
		m_Size = 0;

		if (oldSlots)
		{
			for (std::size_t i = 0; i < oldCapacity; ++i)
			{
				if (oldSlots[i].key != EMPTY_KEY && oldSlots[i].key != TOMBSTONE_KEY)
					InsertInternal(oldSlots[i].key, std::move(oldSlots[i].value));
			}
			delete[] oldSlots;
		}
	}

	Slot* m_Slots{nullptr};
	std::size_t m_Capacity{0};
	std::size_t m_Size{0};
};

} // namespace NN::Runtime::Asset
