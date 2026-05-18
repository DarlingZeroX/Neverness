#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::engine
{
/**
 * @brief 資產登記表（GUID ↔ 虛擬路徑；Phase 5 記憶體實作）。
 */
class AssetRegistrySubsystem final
{
public:
	int RegisterAsset(const char* virtualPathUtf8, NNGuid guid) noexcept;
	int UnregisterByGuid(NNGuid guid) noexcept;
	int UnregisterByPath(const char* virtualPathUtf8) noexcept;
	int ResolvePathByGuid(NNGuid guid, char* outUtf8, std::size_t outCapacity) const noexcept;
	int ResolveGuidByPath(const char* virtualPathUtf8, NNGuid* outGuid) const noexcept;
	std::uint32_t GetDependencyCount(NNGuid guid) const noexcept;
	int GetDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDependency) const noexcept;
	NNGuid ImportAsset(const char* virtualPathUtf8) noexcept;

private:
	static bool GuidIsZero(NNGuid g) noexcept;
	static NNGuid MakeSyntheticGuid(const std::string& path) noexcept;

	mutable std::mutex mutex_{};
	std::unordered_map<std::string, NNGuid> pathToGuid_{};
	std::unordered_map<std::uint64_t, std::string> guidLowToPath_{};
	std::unordered_map<std::uint64_t, std::vector<NNGuid>> dependencies_{};
};
} // namespace visiongal::engine
