/**
 * @file ObjectStubDatabase.cpp
 * @brief **NNObjectAPI** Stub 狀態：handle → 型別名與引用計數（僅本模組可見）。
 */

#include "Object/ObjectStubDatabase.h"

#include <atomic>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>

namespace NN::StubRuntime::Object
{
namespace
{
	std::mutex g_mutex;
	std::atomic<std::uint64_t> g_nextId{1};

	struct StubObjSlot
	{
		std::string typeName{};
		std::uint32_t refs{1};
	};

	std::unordered_map<std::uint64_t, StubObjSlot> g_slots;
} // namespace

NNObjectHandle CreateObject(const char* typeNameUtf8)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	const std::uint64_t id = g_nextId.fetch_add(1u, std::memory_order_relaxed);
	if (id == 0)
	{
		return 0;
	}
	StubObjSlot s{};
	s.typeName = (typeNameUtf8 != nullptr) ? typeNameUtf8 : "";
	s.refs = 1u;
	g_slots[id] = std::move(s);
	return static_cast<NNObjectHandle>(id);
}

void DestroyObject(NNObjectHandle object)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	g_slots.erase(static_cast<std::uint64_t>(object));
}

void RetainObject(NNObjectHandle object)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	const auto it = g_slots.find(static_cast<std::uint64_t>(object));
	if (it != g_slots.end())
	{
		++it->second.refs;
	}
}

void ReleaseObject(NNObjectHandle object)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	const auto it = g_slots.find(static_cast<std::uint64_t>(object));
	if (it == g_slots.end())
	{
		return;
	}
	if (it->second.refs > 0u)
	{
		--it->second.refs;
	}
	if (it->second.refs == 0u)
	{
		g_slots.erase(it);
	}
}

std::uint32_t GetRefCount(NNObjectHandle object)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	const auto it = g_slots.find(static_cast<std::uint64_t>(object));
	return it != g_slots.end() ? it->second.refs : 0u;
}

bool IsAlive(NNObjectHandle object)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	return g_slots.find(static_cast<std::uint64_t>(object)) != g_slots.end();
}

int GetTypeName(NNObjectHandle object, char* outUtf8, std::size_t outCapacity)
{
	if (outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(g_mutex);
	const auto it = g_slots.find(static_cast<std::uint64_t>(object));
	if (it == g_slots.end())
	{
		return -1;
	}
	const std::string& tn = it->second.typeName;
	if (tn.size() + 1u > outCapacity)
	{
		return -1;
	}
	std::memcpy(outUtf8, tn.data(), tn.size());
	outUtf8[tn.size()] = '\0';
	return static_cast<int>(tn.size());
}
} // namespace NN::StubRuntime::Object
