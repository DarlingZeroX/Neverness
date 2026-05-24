#include "NNGuidTable.h"

namespace NN::Runtime::Asset
{

void NNGuidTable::Register(const std::string& path, NNGuid guid)
{
	/* 如果路徑已有舊 GUID，移除舊映射 */
	auto itPath = pathToGuid_.find(path);
	if (itPath != pathToGuid_.end())
	{
		guidToPath_.erase(itPath->second.low);
	}

	/* 如果 GUID 已有舊路徑，移除舊映射 */
	auto itGuid = guidToPath_.find(guid.low);
	if (itGuid != guidToPath_.end())
	{
		pathToGuid_.erase(itGuid->second);
	}

	guidToPath_[guid.low] = path;
	pathToGuid_[path] = guid;
}

std::string NNGuidTable::UnregisterByGuid(NNGuid guid)
{
	auto it = guidToPath_.find(guid.low);
	if (it == guidToPath_.end())
		return {};

	std::string path = std::move(it->second);
	guidToPath_.erase(it);
	pathToGuid_.erase(path);
	return path;
}

NNGuid NNGuidTable::UnregisterByPath(const std::string& path)
{
	auto it = pathToGuid_.find(path);
	if (it == pathToGuid_.end())
	{
		NNGuid z{};
		return z;
	}

	NNGuid guid = it->second;
	pathToGuid_.erase(it);
	guidToPath_.erase(guid.low);
	return guid;
}

bool NNGuidTable::ResolvePath(NNGuid guid, std::string& outPath) const
{
	auto it = guidToPath_.find(guid.low);
	if (it == guidToPath_.end())
		return false;
	outPath = it->second;
	return true;
}

bool NNGuidTable::ResolveGuid(const std::string& path, NNGuid& outGuid) const
{
	auto it = pathToGuid_.find(path);
	if (it == pathToGuid_.end())
		return false;
	outGuid = it->second;
	return true;
}

bool NNGuidTable::ContainsGuid(NNGuid guid) const
{
	return guidToPath_.find(guid.low) != guidToPath_.end();
}

std::uint32_t NNGuidTable::GetCount() const
{
	return static_cast<std::uint32_t>(guidToPath_.size());
}

void NNGuidTable::Clear()
{
	guidToPath_.clear();
	pathToGuid_.clear();
}

} // namespace NN::Runtime::Asset
