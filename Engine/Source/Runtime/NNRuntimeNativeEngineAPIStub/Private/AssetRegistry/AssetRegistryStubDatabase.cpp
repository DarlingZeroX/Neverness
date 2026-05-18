/**
 * @file AssetRegistryStubDatabase.cpp
 * @brief **NNAssetRegistryAPI** Stub 狀態：虛擬路徑與 GUID 雙向映射（僅本模組）。
 */

#include "AssetRegistry/AssetRegistryStubDatabase.h"

#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>

namespace NN::StubRuntime::AssetRegistry
{
namespace
{
	std::mutex g_mutex;
	std::unordered_map<std::string, NNGuid> g_pathToGuid;
	std::unordered_map<std::uint64_t, std::string> g_guidLowToPath;
} // namespace

int RegisterAsset(const char* virtualPathUtf8, NNGuid guid)
{
	if (virtualPathUtf8 == nullptr)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(g_mutex);
	const std::string p(virtualPathUtf8);
	g_pathToGuid[p] = guid;
	g_guidLowToPath[guid.low] = p;
	return 0;
}

int UnregisterByGuid(NNGuid guid)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	const auto it = g_guidLowToPath.find(guid.low);
	if (it == g_guidLowToPath.end())
	{
		return -1;
	}
	g_pathToGuid.erase(it->second);
	g_guidLowToPath.erase(it);
	return 0;
}

int UnregisterByPath(const char* virtualPathUtf8)
{
	if (virtualPathUtf8 == nullptr)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(g_mutex);
	const std::string p(virtualPathUtf8);
	const auto it = g_pathToGuid.find(p);
	if (it == g_pathToGuid.end())
	{
		return -1;
	}
	g_guidLowToPath.erase(it->second.low);
	g_pathToGuid.erase(it);
	return 0;
}

int ResolvePathByGuid(NNGuid guid, char* outUtf8, std::size_t outCapacity)
{
	if (outUtf8 == nullptr || outCapacity == 0u)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(g_mutex);
	const auto it = g_guidLowToPath.find(guid.low);
	if (it == g_guidLowToPath.end())
	{
		return -1;
	}
	const std::string& path = it->second;
	if (path.size() + 1u > outCapacity)
	{
		return -1;
	}
	std::memcpy(outUtf8, path.data(), path.size());
	outUtf8[path.size()] = '\0';
	return static_cast<int>(path.size());
}

int ResolveGuidByPath(const char* virtualPathUtf8, NNGuid* outGuid)
{
	if (virtualPathUtf8 == nullptr || outGuid == nullptr)
	{
		return -1;
	}
	std::lock_guard<std::mutex> lock(g_mutex);
	const auto it = g_pathToGuid.find(std::string(virtualPathUtf8));
	if (it == g_pathToGuid.end())
	{
		return -1;
	}
	*outGuid = it->second;
	return 0;
}

std::uint32_t GetDependencyCount(NNGuid guid)
{
	(void)guid;
	return 0u;
}

int GetDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDependency)
{
	(void)guid;
	(void)index;
	(void)outDependency;
	return -1;
}

NNGuid ImportAsset(const char* virtualPathUtf8)
{
	NNGuid z{};
	if (virtualPathUtf8 == nullptr)
	{
		return z;
	}
	z.high = 1u;
	z.low = 2u;
	(void)RegisterAsset(virtualPathUtf8, z);
	return z;
}
} // namespace NN::StubRuntime::AssetRegistry
