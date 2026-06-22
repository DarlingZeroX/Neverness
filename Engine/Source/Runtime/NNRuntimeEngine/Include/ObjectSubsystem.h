#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "Engine/EngineHandles.h"

namespace NN::Runtime::engine
{
/**
 * @brief 託管 VGObject 之 Native 端引用計數表（Phase 5）。
 */
class ObjectSubsystem final
{
public:
	NNObjectHandle CreateObject(const char* typeNameUtf8) noexcept;
	void DestroyObject(NNObjectHandle object) noexcept;
	void RetainObject(NNObjectHandle object) noexcept;
	void ReleaseObject(NNObjectHandle object) noexcept;
	std::uint32_t GetRefCount(NNObjectHandle object) const noexcept;
	int IsAlive(NNObjectHandle object) const noexcept;
	int GetTypeName(NNObjectHandle object, char* outUtf8, std::size_t outCapacity) const noexcept;

private:
	struct Slot
	{
		std::string typeName{};
		std::uint32_t refCount{1};
		bool alive{true};
	};

	mutable std::mutex mutex_{};
	std::unordered_map<NNObjectHandle, Slot> slots_{};
	std::atomic<NNObjectHandle> nextHandle_{1};
};
} // namespace visiongal::engine
