#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "NNNativeEngineAPI/Include/EngineHandles.h"
#include "NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::engine
{
/**
 * @brief 場景子系統：Phase 5 提供最小 **實體層級 / 變換 / 命名** 記憶體模型（未接 VGEngine）。
 */
class SceneSubsystem final
{
public:
	int LoadScene(const char* sceneNameUtf8) noexcept;
	NNEntityHandle Spawn(const char* prefabVirtualPathUtf8) noexcept;
	void Destroy(NNEntityHandle entity) noexcept;
	NNEntityHandle Find(const char* entityNameUtf8) noexcept;
	void Activate(NNEntityHandle entity, int active) noexcept;

	int UnloadScene(const char* sceneNameUtf8) noexcept;
	int GetActiveSceneName(char* outUtf8, std::size_t outCapacity) const noexcept;
	void SetParent(NNEntityHandle child, NNEntityHandle parent) noexcept;
	NNEntityHandle GetParent(NNEntityHandle entity) const noexcept;
	std::uint32_t GetChildCount(NNEntityHandle entity) const noexcept;
	NNEntityHandle GetChildAt(NNEntityHandle entity, std::uint32_t index) const noexcept;
	void GetTransform(NNEntityHandle entity, NNTransform3* outTransform) const noexcept;
	void SetTransform(NNEntityHandle entity, const NNTransform3* transform) noexcept;
	int SetEntityName(NNEntityHandle entity, const char* nameUtf8) noexcept;
	int GetEntityName(NNEntityHandle entity, char* outUtf8, std::size_t outCapacity) const noexcept;

private:
	struct Entity
	{
		NNEntityHandle id{0};
		std::string name{};
		NNEntityHandle parent{0};
		std::vector<NNEntityHandle> children{};
		NNTransform3 transform{};
		int active{1};
		bool alive{true};
	};

	static NNTransform3 DefaultTransform() noexcept;

	void RemoveFromParent(NNEntityHandle child) noexcept;
	NNEntityHandle AllocateEntity() noexcept;

	mutable std::mutex mutex_{};
	std::string activeScene_{};
	std::unordered_map<NNEntityHandle, Entity> entities_{};
	std::atomic<NNEntityHandle> nextEntity_{1};
};
} // namespace visiongal::engine
