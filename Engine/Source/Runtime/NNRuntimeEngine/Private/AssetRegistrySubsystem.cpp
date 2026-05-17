#include "AssetRegistrySubsystem.h"

#include <cstring>

namespace NN::Runtime::engine
{
namespace
{
	std::uint64_t HashPath(const std::string& s) noexcept
	{
		// FNV-1a 64-bit
		std::uint64_t h = 14695981039346656037ull;
		for (unsigned char c : s)
		{
			h ^= static_cast<std::uint64_t>(c);
			h *= 1099511628211ull;
		}
		return h;
	}
} // namespace

bool AssetRegistrySubsystem::GuidIsZero(VGGuid g) noexcept
{
	return g.high == 0u && g.low == 0u;
}

VGGuid AssetRegistrySubsystem::MakeSyntheticGuid(const std::string& path) noexcept
{
	const std::uint64_t h = HashPath(path);
	VGGuid g{};
	g.high = 0x56474153u; // 'VGAS' 魔數前綴
	g.low = h == 0u ? 1u : h;
	return g;
}

int AssetRegistrySubsystem::RegisterAsset(const char* virtualPathUtf8, VGGuid guid) noexcept
{
	if (virtualPathUtf8 == nullptr || GuidIsZero(guid))
	{
		return -1;
	}
	const std::string path(virtualPathUtf8);
	std::lock_guard<std::mutex> lock(mutex_);
	pathToGuid_[path] = guid;
	guidLowToPath_[guid.low] = path;
	if (dependencies_.find(guid.low) == dependencies_.end())
	{
		dependencies_[guid.low] = {};
	}
	return 0;
}

int AssetRegistrySubsystem::UnregisterByGuid(VGGuid guid) noexcept
{
	if (GuidIsZero(guid))
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto itPath = guidLowToPath_.find(guid.low);
	if (itPath == guidLowToPath_.end())
	{
		return -1;
	}
	pathToGuid_.erase(itPath->second);
	guidLowToPath_.erase(itPath);
	dependencies_.erase(guid.low);
	return 0;
}

int AssetRegistrySubsystem::UnregisterByPath(const char* virtualPathUtf8) noexcept
{
	if (virtualPathUtf8 == nullptr)
	{
		return -1;
	}
	const std::string path(virtualPathUtf8);
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = pathToGuid_.find(path);
	if (it == pathToGuid_.end())
	{
		return -1;
	}
	const VGGuid g = it->second;
	guidLowToPath_.erase(g.low);
	pathToGuid_.erase(it);
	dependencies_.erase(g.low);
	return 0;
}

int AssetRegistrySubsystem::ResolvePathByGuid(VGGuid guid, char* outUtf8, std::size_t outCapacity) const noexcept
{
	if (outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = guidLowToPath_.find(guid.low);
	if (it == guidLowToPath_.end())
	{
		return -1;
	}
	const std::string& p = it->second;
	if (p.size() + 1u > outCapacity)
	{
		return -1;
	}
	std::memcpy(outUtf8, p.data(), p.size());
	outUtf8[p.size()] = '\0';
	return static_cast<int>(p.size());
}

int AssetRegistrySubsystem::ResolveGuidByPath(const char* virtualPathUtf8, VGGuid* outGuid) const noexcept
{
	if (virtualPathUtf8 == nullptr || outGuid == nullptr)
	{
		return -1;
	}
	const std::string path(virtualPathUtf8);
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = pathToGuid_.find(path);
	if (it == pathToGuid_.end())
	{
		return -1;
	}
	*outGuid = it->second;
	return 0;
}

std::uint32_t AssetRegistrySubsystem::GetDependencyCount(VGGuid guid) const noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = dependencies_.find(guid.low);
	if (it == dependencies_.end())
	{
		return 0u;
	}
	return static_cast<std::uint32_t>(it->second.size());
}

int AssetRegistrySubsystem::GetDependencyAt(VGGuid guid, std::uint32_t index, VGGuid* outDependency) const noexcept
{
	if (outDependency == nullptr)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = dependencies_.find(guid.low);
	if (it == dependencies_.end() || index >= it->second.size())
	{
		return -1;
	}
	*outDependency = it->second[static_cast<std::size_t>(index)];
	return 0;
}

VGGuid AssetRegistrySubsystem::ImportAsset(const char* virtualPathUtf8) noexcept
{
	if (virtualPathUtf8 == nullptr)
	{
		VGGuid z{};
		return z;
	}
	const std::string path(virtualPathUtf8);
	const VGGuid g = MakeSyntheticGuid(path);
	(void)RegisterAsset(virtualPathUtf8, g);
	return g;
}
} // namespace visiongal::engine
