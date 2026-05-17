#include "ObjectSubsystem.h"

#include <algorithm>
#include <cstring>

namespace NN::Runtime::engine
{
VGObjectHandle ObjectSubsystem::CreateObject(const char* typeNameUtf8) noexcept
{
	const char* const tn = (typeNameUtf8 != nullptr) ? typeNameUtf8 : "";
	std::lock_guard<std::mutex> lock(mutex_);
	const VGObjectHandle id = nextHandle_.fetch_add(1u, std::memory_order_relaxed);
	if (id == 0)
	{
		return 0;
	}
	Slot slot{};
	slot.typeName = tn;
	slot.refCount = 1u;
	slot.alive = true;
	slots_.emplace(id, std::move(slot));
	return id;
}

void ObjectSubsystem::DestroyObject(VGObjectHandle object) noexcept
{
	if (object == 0)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	slots_.erase(object);
}

void ObjectSubsystem::RetainObject(VGObjectHandle object) noexcept
{
	if (object == 0)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = slots_.find(object);
	if (it == slots_.end())
	{
		return;
	}
	++it->second.refCount;
}

void ObjectSubsystem::ReleaseObject(VGObjectHandle object) noexcept
{
	if (object == 0)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = slots_.find(object);
	if (it == slots_.end())
	{
		return;
	}
	if (it->second.refCount > 0u)
	{
		--it->second.refCount;
	}
	if (it->second.refCount == 0u)
	{
		slots_.erase(it);
	}
}

std::uint32_t ObjectSubsystem::GetRefCount(VGObjectHandle object) const noexcept
{
	if (object == 0)
	{
		return 0u;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = slots_.find(object);
	if (it == slots_.end())
	{
		return 0u;
	}
	return it->second.refCount;
}

int ObjectSubsystem::IsAlive(VGObjectHandle object) const noexcept
{
	if (object == 0)
	{
		return 0;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = slots_.find(object);
	if (it == slots_.end() || !it->second.alive)
	{
		return 0;
	}
	return 1;
}

int ObjectSubsystem::GetTypeName(VGObjectHandle object, char* outUtf8, std::size_t outCapacity) const noexcept
{
	if (object == 0 || outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = slots_.find(object);
	if (it == slots_.end())
	{
		return -1;
	}
	const std::string& tn = it->second.typeName;
	const std::size_t n = tn.size();
	if (n + 1u > outCapacity)
	{
		return -1;
	}
	std::memcpy(outUtf8, tn.data(), n);
	outUtf8[n] = '\0';
	return static_cast<int>(n);
}
} // namespace visiongal::engine
