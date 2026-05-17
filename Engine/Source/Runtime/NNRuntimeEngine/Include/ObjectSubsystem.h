#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "NNNativeEngineAPI/Include/EngineHandles.h"

namespace NN::Runtime::engine
{
/**
 * @brief 託管 VGObject 之 Native 端引用計數表（Phase 5）。
 */
class ObjectSubsystem final
{
public:
	VGObjectHandle CreateObject(const char* typeNameUtf8) noexcept;
	void DestroyObject(VGObjectHandle object) noexcept;
	void RetainObject(VGObjectHandle object) noexcept;
	void ReleaseObject(VGObjectHandle object) noexcept;
	std::uint32_t GetRefCount(VGObjectHandle object) const noexcept;
	int IsAlive(VGObjectHandle object) const noexcept;
	int GetTypeName(VGObjectHandle object, char* outUtf8, std::size_t outCapacity) const noexcept;

private:
	struct Slot
	{
		std::string typeName{};
		std::uint32_t refCount{1};
		bool alive{true};
	};

	mutable std::mutex mutex_{};
	std::unordered_map<VGObjectHandle, Slot> slots_{};
	std::atomic<VGObjectHandle> nextHandle_{1};
};
} // namespace visiongal::engine
