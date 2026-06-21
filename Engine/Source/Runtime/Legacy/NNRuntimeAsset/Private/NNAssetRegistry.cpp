#include "NNAssetRegistry.h"

#include <cstring>
#include <iostream>
#include <ostream>

namespace NN::Runtime::Asset
{

NNAssetRegistry& NNAssetRegistry::Instance() noexcept
{
	static NNAssetRegistry instance;
	return instance;
}

/* ======================== GUID ↔ Path ======================== */

int NNAssetRegistry::RegisterAsset(const char* virtualPathUtf8, NNGuid guid) noexcept
{
	if (virtualPathUtf8 == nullptr || GuidIsZero(guid))
	{
		std::cerr << "[NNAssetRegistry] RegisterAsset: null path or zero guid" << std::endl;
		return -1;
	}

	std::lock_guard<std::mutex> lock(mutex_);
	guidTable_.Register(virtualPathUtf8, guid);
	std::cout << "[NNAssetRegistry] RegisterAsset: path=" << virtualPathUtf8
		<< " guid=(" << guid.high << ":" << guid.low << ")" << std::endl;
	return 0;
}

int NNAssetRegistry::UnregisterByGuid(NNGuid guid) noexcept
{
	if (GuidIsZero(guid))
		return -1;

	std::lock_guard<std::mutex> lock(mutex_);
	std::string path = guidTable_.UnregisterByGuid(guid);
	if (path.empty())
		return -1;

	depTable_.ClearDependencies(guid);
	return 0;
}

int NNAssetRegistry::UnregisterByPath(const char* virtualPathUtf8) noexcept
{
	if (virtualPathUtf8 == nullptr)
		return -1;

	std::lock_guard<std::mutex> lock(mutex_);
	NNGuid guid = guidTable_.UnregisterByPath(virtualPathUtf8);
	if (GuidIsZero(guid))
		return -1;

	depTable_.ClearDependencies(guid);
	return 0;
}

int NNAssetRegistry::ResolvePathByGuid(NNGuid guid, char* outUtf8, std::size_t outCapacity) const noexcept
{
	if (outUtf8 == nullptr || outCapacity == 0)
		return -1;

	std::lock_guard<std::mutex> lock(mutex_);
	std::string path;
	if (!guidTable_.ResolvePath(guid, path))
		return -1;

	if (path.size() + 1 > outCapacity)
		return -1;

	std::memcpy(outUtf8, path.data(), path.size());
	outUtf8[path.size()] = '\0';
	return static_cast<int>(path.size());
}

int NNAssetRegistry::ResolveGuidByPath(const char* virtualPathUtf8, NNGuid* outGuid) const noexcept
{
	if (virtualPathUtf8 == nullptr || outGuid == nullptr)
		return -1;

	std::lock_guard<std::mutex> lock(mutex_);
	return guidTable_.ResolveGuid(virtualPathUtf8, *outGuid) ? 0 : -1;
}

/* ======================== 依賴管理 ======================== */

int NNAssetRegistry::SetDependencies(NNGuid guid, const NNGuid* deps, std::uint32_t count) noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	depTable_.SetDependencies(guid, deps, count);
	return 0;
}

int NNAssetRegistry::AddDependency(NNGuid guid, NNGuid dependency) noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	depTable_.AddDependency(guid, dependency);
	return 0;
}

int NNAssetRegistry::RemoveDependency(NNGuid guid, NNGuid dependency) noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	depTable_.RemoveDependency(guid, dependency);
	return 0;
}

std::uint32_t NNAssetRegistry::GetDependencyCount(NNGuid guid) const noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	return depTable_.GetDependencyCount(guid);
}

int NNAssetRegistry::GetDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDependency) const noexcept
{
	if (outDependency == nullptr)
		return -1;

	std::lock_guard<std::mutex> lock(mutex_);
	return depTable_.GetDependencyAt(guid, index, *outDependency) ? 0 : -1;
}

std::uint32_t NNAssetRegistry::GetReverseDependencyCount(NNGuid guid) const noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	return depTable_.GetReverseDependencyCount(guid);
}

int NNAssetRegistry::GetReverseDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDep) const noexcept
{
	if (outDep == nullptr)
		return -1;

	std::lock_guard<std::mutex> lock(mutex_);
	return depTable_.GetReverseDependencyAt(guid, index, *outDep) ? 0 : -1;
}

int NNAssetRegistry::HasCycle() const noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	return depTable_.HasCycle() ? 1 : 0;
}

std::uint32_t NNAssetRegistry::GetAssetCount() const noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	return guidTable_.GetCount();
}

std::uint32_t NNAssetRegistry::GetEdgeCount() const noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	return depTable_.GetEdgeCount();
}

/* ======================== 匯入 ======================== */

NNGuid NNAssetRegistry::ImportAsset(const char* virtualPathUtf8) noexcept
{
	if (virtualPathUtf8 == nullptr)
	{
		NNGuid z{};
		return z;
	}

	const std::string path(virtualPathUtf8);
	const NNGuid g = MakeSyntheticGuid(path);

	std::lock_guard<std::mutex> lock(mutex_);
	guidTable_.Register(path, g);
	return g;
}

/* ======================== 內部工具 ======================== */

bool NNAssetRegistry::GuidIsZero(NNGuid g) noexcept
{
	return g.high == 0 && g.low == 0;
}

std::uint64_t NNAssetRegistry::HashPath(const std::string& s) noexcept
{
	std::uint64_t h = 14695981039346656037ull;
	for (unsigned char c : s)
	{
		h ^= static_cast<std::uint64_t>(c);
		h *= 1099511628211ull;
	}
	return h;
}

NNGuid NNAssetRegistry::MakeSyntheticGuid(const std::string& path) noexcept
{
	const std::uint64_t h = HashPath(path);
	NNGuid g{};
	g.high = 0x4E4E4153ull; /* 'NNAS' 魔數前綴 */
	g.low = h == 0 ? 1 : h;
	return g;
}

} // namespace NN::Runtime::Asset
