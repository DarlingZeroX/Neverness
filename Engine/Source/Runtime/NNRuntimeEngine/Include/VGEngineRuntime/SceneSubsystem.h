#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "NNNativeEngineAPI/EngineHandles.h"
#include "NNNativeEngineAPI/EngineTypes.h"

namespace visiongal::engine
{
/**
 * @brief 場景子系統：Phase 5 提供最小 **實體層級 / 變換 / 命名** 記憶體模型（未接 VGEngine）。
 */
class SceneSubsystem final
{
public:
	int LoadScene(const char* sceneNameUtf8) noexcept;
	VGEntityHandle Spawn(const char* prefabVirtualPathUtf8) noexcept;
	void Destroy(VGEntityHandle entity) noexcept;
	VGEntityHandle Find(const char* entityNameUtf8) noexcept;
	void Activate(VGEntityHandle entity, int active) noexcept;

	int UnloadScene(const char* sceneNameUtf8) noexcept;
	int GetActiveSceneName(char* outUtf8, std::size_t outCapacity) const noexcept;
	void SetParent(VGEntityHandle child, VGEntityHandle parent) noexcept;
	VGEntityHandle GetParent(VGEntityHandle entity) const noexcept;
	std::uint32_t GetChildCount(VGEntityHandle entity) const noexcept;
	VGEntityHandle GetChildAt(VGEntityHandle entity, std::uint32_t index) const noexcept;
	void GetTransform(VGEntityHandle entity, VGTransform3* outTransform) const noexcept;
	void SetTransform(VGEntityHandle entity, const VGTransform3* transform) noexcept;
	int SetEntityName(VGEntityHandle entity, const char* nameUtf8) noexcept;
	int GetEntityName(VGEntityHandle entity, char* outUtf8, std::size_t outCapacity) const noexcept;

private:
	struct Entity
	{
		VGEntityHandle id{0};
		std::string name{};
		VGEntityHandle parent{0};
		std::vector<VGEntityHandle> children{};
		VGTransform3 transform{};
		int active{1};
		bool alive{true};
	};

	static VGTransform3 DefaultTransform() noexcept;

	void RemoveFromParent(VGEntityHandle child) noexcept;
	VGEntityHandle AllocateEntity() noexcept;

	mutable std::mutex mutex_{};
	std::string activeScene_{};
	std::unordered_map<VGEntityHandle, Entity> entities_{};
	std::atomic<VGEntityHandle> nextEntity_{1};
};
} // namespace visiongal::engine
