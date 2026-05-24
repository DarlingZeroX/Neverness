#include "NNCookManifest.h"

namespace NN::Runtime::Asset
{

void NNCookManifest::AddAsset(const NNCookAssetEntry& entry)
{
	assets_.push_back(entry);
}

void NNCookManifest::AddGroup(const NNCookGroup& group)
{
	groups_.push_back(group);
}

void NNCookManifest::SetOutputRoot(const std::string& path)
{
	outputRoot_ = path;
}

void NNCookManifest::SetLibraryRoot(const std::string& path)
{
	libraryRoot_ = path;
}

void NNCookManifest::Clear()
{
	assets_.clear();
	groups_.clear();
}

} // namespace NN::Runtime::Asset
