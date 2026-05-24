#include "NNAssetFormat.h"

#include <cstring>
#include <fstream>
#include <vector>

namespace NN::Runtime::Asset
{

bool ReadAssetHeader(const std::string& path, NNAssetHeader& outHeader)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
		return false;

	file.read(reinterpret_cast<char*>(&outHeader), sizeof(NNAssetHeader));
	if (!file)
		return false;

	return NNAssetHeaderIsValid(&outHeader) != 0;
}

bool ReadAssetFile(const std::string& path,
                   NNAssetHeader& outHeader,
                   std::vector<NNGuid>& outDeps,
                   std::vector<NNBlobDescriptor>& outBlobs,
                   std::vector<std::uint8_t>& outPayload)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
		return false;

	const auto fileSize = static_cast<std::size_t>(file.tellg());
	file.seekg(0);

	/* 讀取 Header */
	if (fileSize < sizeof(NNAssetHeader))
		return false;

	file.read(reinterpret_cast<char*>(&outHeader), sizeof(NNAssetHeader));
	if (NNAssetHeaderIsValid(&outHeader) == 0)
		return false;

	/* 讀取依賴 GUID */
	outDeps.resize(outHeader.dependencyCount);
	if (outHeader.dependencyCount > 0)
	{
		file.seekg(static_cast<std::streamoff>(outHeader.dependencyOffset));
		file.read(reinterpret_cast<char*>(outDeps.data()),
		          static_cast<std::streamsize>(outHeader.dependencyCount * sizeof(NNGuid)));
	}

	/* 讀取 Blob 描述符 */
	outBlobs.resize(outHeader.blobCount);
	if (outHeader.blobCount > 0)
	{
		file.seekg(static_cast<std::streamoff>(outHeader.blobTableOffset));
		file.read(reinterpret_cast<char*>(outBlobs.data()),
		          static_cast<std::streamsize>(outHeader.blobCount * sizeof(NNBlobDescriptor)));
	}

	/* 讀取 Payload */
	if (outHeader.payloadSize > 0)
	{
		outPayload.resize(static_cast<std::size_t>(outHeader.payloadSize));
		file.seekg(static_cast<std::streamoff>(outHeader.payloadOffset));
		file.read(reinterpret_cast<char*>(outPayload.data()),
		          static_cast<std::streamsize>(outHeader.payloadSize));
	}

	return static_cast<bool>(file);
}

bool WriteAssetFile(const std::string& path,
                    const NNGuid& guid,
                    std::uint64_t typeId,
                    const std::vector<NNGuid>& deps,
                    const std::vector<std::uint8_t>& blobsRaw,
                    std::uint32_t blobCount,
                    const std::vector<std::uint8_t>& payload,
                    std::uint32_t flags)
{
	std::ofstream file(path, std::ios::binary);
	if (!file.is_open())
		return false;

	/* 計算偏移 */
	const std::uint64_t depOffset = sizeof(NNAssetHeader);
	const std::uint64_t depSize = deps.size() * sizeof(NNGuid);
	const std::uint64_t blobTableOffset = depOffset + depSize;
	const std::uint64_t blobTableSize = blobsRaw.size();
	const std::uint64_t payloadOffset = NNAssetAlign(blobTableOffset + blobTableSize);

	/* 填寫 Header */
	NNAssetHeader header{};
	header.magic = NN_ASSET_MAGIC;
	header.version = NN_ASSET_VERSION;
	header.assetGuid = guid;
	header.typeId = typeId;
	header.dependencyCount = static_cast<std::uint32_t>(deps.size());
	header.blobCount = blobCount;
	header.dependencyOffset = depOffset;
	header.blobTableOffset = blobTableOffset;
	header.payloadOffset = payloadOffset;
	header.payloadSize = payload.size();
	header.flags = flags;

	/* 寫入 Header */
	file.write(reinterpret_cast<const char*>(&header), sizeof(NNAssetHeader));

	/* 寫入依賴 */
	if (!deps.empty())
	{
		file.write(reinterpret_cast<const char*>(deps.data()),
		           static_cast<std::streamsize>(depSize));
	}

	/* 寫入 Blob 表 */
	if (!blobsRaw.empty())
	{
		file.write(reinterpret_cast<const char*>(blobsRaw.data()),
		           static_cast<std::streamsize>(blobTableSize));
	}

	/* 對齊填充 */
	const std::uint64_t currentOffset = blobTableOffset + blobTableSize;
	if (currentOffset < payloadOffset)
	{
		const std::uint64_t padSize = payloadOffset - currentOffset;
		std::vector<std::uint8_t> padding(static_cast<std::size_t>(padSize), 0);
		file.write(reinterpret_cast<const char*>(padding.data()),
		           static_cast<std::streamsize>(padSize));
	}

	/* 寫入 Payload */
	if (!payload.empty())
	{
		file.write(reinterpret_cast<const char*>(payload.data()),
		           static_cast<std::streamsize>(payload.size()));
	}

	return static_cast<bool>(file);
}

} // namespace NN::Runtime::Asset
