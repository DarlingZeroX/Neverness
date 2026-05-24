#include "NNPackBuilder.h"

#include <cstring>
#include <filesystem>
#include <fstream>

namespace NN::Runtime::Asset
{

void NNPackBuilder::SetOutputPath(const std::string& path)
{
	outputPath_ = path;
}

void NNPackBuilder::AddAsset(NNGuid guid, std::uint64_t typeId,
                              const std::vector<std::uint8_t>& data)
{
	PendingAsset pa;
	pa.guid = guid;
	pa.typeId = typeId;
	pa.data = data;
	assets_.push_back(std::move(pa));
}

void NNPackBuilder::SetPackageName(const std::string& name)
{
	packageName_ = name;
}

std::uint32_t NNPackBuilder::GetAssetCount() const
{
	return static_cast<std::uint32_t>(assets_.size());
}

void NNPackBuilder::Clear()
{
	assets_.clear();
}

bool NNPackBuilder::Build()
{
	if (outputPath_.empty() || assets_.empty())
		return false;

	/* 确保输出目录存在 */
	auto dir = std::filesystem::path(outputPath_).parent_path();
	if (!dir.empty() && !std::filesystem::exists(dir))
		std::filesystem::create_directories(dir);

	/* 计算各段偏移 */
	const auto headerSize = static_cast<std::uint64_t>(NN_PACK_HEADER_SIZE);
	const auto tableSize = static_cast<std::uint64_t>(assets_.size()) * sizeof(NNPackAssetEntry);
	const auto tableOffset = headerSize;

	/* Manifest（简单格式：包名 + 占位） */
	std::vector<std::uint8_t> manifestData;
	if (!packageName_.empty())
		manifestData.assign(packageName_.begin(), packageName_.end());

	const auto manifestOffset = NNPackAlign(tableOffset + tableSize);
	const auto manifestSize = manifestData.size();
	const auto dataOffset = NNPackAlign(manifestOffset + manifestSize);

	/* 计算数据总大小 */
	std::uint64_t totalDataSize = 0;
	for (const auto& asset : assets_)
		totalDataSize += NNPackAlign(asset.data.size());

	/* 构建 Header */
	NNPackHeader header{};
	header.magic = NN_PACK_MAGIC;
	header.version = NN_PACK_VERSION;
	header.assetCount = static_cast<std::uint32_t>(assets_.size());
	header.flags = 0;
	header.tableOffset = tableOffset;
	header.tableSize = tableSize;
	header.manifestOffset = manifestOffset;
	header.manifestSize = manifestSize;
	header.dataOffset = dataOffset;
	header.totalDataSize = totalDataSize;

	/* 构建 AssetTable */
	std::vector<NNPackAssetEntry> table;
	table.reserve(assets_.size());

	std::uint64_t currentOffset = 0;
	for (const auto& asset : assets_)
	{
		NNPackAssetEntry entry{};
		entry.guidHigh = asset.guid.high;
		entry.guidLow = asset.guid.low;
		entry.typeId = asset.typeId;
		entry.offset = currentOffset;
		entry.size = asset.data.size();
		entry.compressedSize = 0; /* 未压缩 */
		entry.flags = 0;
		table.push_back(entry);

		currentOffset += NNPackAlign(asset.data.size());
	}

	/* 写入文件 */
	std::ofstream file(outputPath_, std::ios::binary);
	if (!file.is_open())
		return false;

	/* Header */
	file.write(reinterpret_cast<const char*>(&header), sizeof(NNPackHeader));

	/* 填充对齐 */
	{
		auto pad = static_cast<std::streamsize>(tableOffset - sizeof(NNPackHeader));
		if (pad > 0)
		{
			std::vector<char> padding(pad, 0);
			file.write(padding.data(), pad);
		}
	}

	/* AssetTable */
	file.write(reinterpret_cast<const char*>(table.data()),
	           static_cast<std::streamsize>(tableSize));

	/* 填充对齐到 Manifest */
	{
		auto pad = static_cast<std::streamsize>(manifestOffset - (tableOffset + tableSize));
		if (pad > 0)
		{
			std::vector<char> padding(pad, 0);
			file.write(padding.data(), pad);
		}
	}

	/* Manifest */
	if (!manifestData.empty())
		file.write(reinterpret_cast<const char*>(manifestData.data()),
		           static_cast<std::streamsize>(manifestSize));

	/* 填充对齐到 Data */
	{
		auto pad = static_cast<std::streamsize>(dataOffset - (manifestOffset + manifestSize));
		if (pad > 0)
		{
			std::vector<char> padding(pad, 0);
			file.write(padding.data(), pad);
		}
	}

	/* Asset Data */
	for (const auto& asset : assets_)
	{
		file.write(reinterpret_cast<const char*>(asset.data.data()),
		           static_cast<std::streamsize>(asset.data.size()));

		/* 64 字节对齐填充 */
		auto aligned = NNPackAlign(asset.data.size());
		auto pad = static_cast<std::streamsize>(aligned - asset.data.size());
		if (pad > 0)
		{
			std::vector<char> padding(pad, 0);
			file.write(padding.data(), pad);
		}
	}

	return file.good();
}

} // namespace NN::Runtime::Asset
