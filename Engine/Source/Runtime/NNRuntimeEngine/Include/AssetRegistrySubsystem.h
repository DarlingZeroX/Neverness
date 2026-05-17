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
	int RegisterAsset(const char* virtualPathUtf8, VGGuid guid) noexcept;
	int UnregisterByGuid(VGGuid guid) noexcept;
	int UnregisterByPath(const char* virtualPathUtf8) noexcept;
	int ResolvePathByGuid(VGGuid guid, char* outUtf8, std::size_t outCapacity) const noexcept;
	int ResolveGuidByPath(const char* virtualPathUtf8, VGGuid* outGuid) const noexcept;
	std::uint32_t GetDependencyCount(VGGuid guid) const noexcept;
	int GetDependencyAt(VGGuid guid, std::uint32_t index, VGGuid* outDependency) const noexcept;
	VGGuid ImportAsset(const char* virtualPathUtf8) noexcept;

private:
	static bool GuidIsZero(VGGuid g) noexcept;
	static VGGuid MakeSyntheticGuid(const std::string& path) noexcept;

	mutable std::mutex mutex_{};
	std::unordered_map<std::string, VGGuid> pathToGuid_{};
	std::unordered_map<std::uint64_t, std::string> guidLowToPath_{};
	std::unordered_map<std::uint64_t, std::vector<VGGuid>> dependencies_{};
};
} // namespace visiongal::engine
