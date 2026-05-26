#include "NNAssetFormat.h"

#include <cstring>
#include <NNRuntimeVFS/Include/VFSService.h>
#include <vector>

namespace NN::Runtime::Asset
{

bool ReadAssetHeader(const std::string& path, NNAssetHeader& outHeader)
{
	namespace VFS = NN::Runtime::VFS;
	auto file = VFS::VFSService::GetInstance()->OpenFile(
		VFS::FileInfo(path), VFS::IFile::FileMode::Read);
	if (!file || !file->IsOpened())
		return false;

	file->Read(reinterpret_cast<uint8_t*>(&outHeader), sizeof(NNAssetHeader));
	file->Close();

	return NNAssetHeaderIsValid(&outHeader) != 0;
}

bool ReadAssetFile(const std::string& path,
                   NNAssetHeader& outHeader,
                   std::vector<NNGuid>& outDeps,
                   std::vector<NNBlobDescriptor>& outBlobs,
                   std::vector<std::uint8_t>& outPayload)
{
	namespace VFS = NN::Runtime::VFS;
	auto file = VFS::VFSService::GetInstance()->OpenFile(
		VFS::FileInfo(path), VFS::IFile::FileMode::Read);
	if (!file || !file->IsOpened())
		return false;

	const auto fileSize = static_cast<std::size_t>(file->Size());

	/* 讀取 Header */
	if (fileSize < sizeof(NNAssetHeader))
		return false;

	file->Read(reinterpret_cast<uint8_t*>(&outHeader), sizeof(NNAssetHeader));
	if (NNAssetHeaderIsValid(&outHeader) == 0)
		return false;

	/* 讀取依賴 GUID */
	outDeps.resize(outHeader.dependencyCount);
	if (outHeader.dependencyCount > 0)
	{
		file->Seek(outHeader.dependencyOffset, VFS::IFile::Origin::Begin);
		file->Read(reinterpret_cast<uint8_t*>(outDeps.data()),
		           outHeader.dependencyCount * sizeof(NNGuid));
	}

	/* 讀取 Blob 描述符 */
	outBlobs.resize(outHeader.blobCount);
	if (outHeader.blobCount > 0)
	{
		file->Seek(outHeader.blobTableOffset, VFS::IFile::Origin::Begin);
		file->Read(reinterpret_cast<uint8_t*>(outBlobs.data()),
		           outHeader.blobCount * sizeof(NNBlobDescriptor));
	}

	/* 讀取 Payload */
	if (outHeader.payloadSize > 0)
	{
		outPayload.resize(static_cast<std::size_t>(outHeader.payloadSize));
		file->Seek(outHeader.payloadOffset, VFS::IFile::Origin::Begin);
		file->Read(reinterpret_cast<uint8_t*>(outPayload.data()),
		           outHeader.payloadSize);
	}

	file->Close();
	return true;
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
	/* 先在內存中構建完整文件，再通過 VFS 寫入 */
	std::vector<std::uint8_t> buffer;

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
	buffer.insert(buffer.end(), reinterpret_cast<std::uint8_t*>(&header),
	              reinterpret_cast<std::uint8_t*>(&header) + sizeof(NNAssetHeader));

	/* 寫入依賴 */
	if (!deps.empty())
	{
		buffer.insert(buffer.end(), reinterpret_cast<const std::uint8_t*>(deps.data()),
		              reinterpret_cast<const std::uint8_t*>(deps.data()) + depSize);
	}

	/* 寫入 Blob 表 */
	if (!blobsRaw.empty())
	{
		buffer.insert(buffer.end(), reinterpret_cast<const std::uint8_t*>(blobsRaw.data()),
		              reinterpret_cast<const std::uint8_t*>(blobsRaw.data()) + blobTableSize);
	}

	/* 對齊填充 */
	const std::uint64_t currentOffset = blobTableOffset + blobTableSize;
	if (currentOffset < payloadOffset)
	{
		const std::uint64_t padSize = payloadOffset - currentOffset;
		std::vector<std::uint8_t> padding(static_cast<std::size_t>(padSize), 0);
		buffer.insert(buffer.end(), padding.begin(), padding.end());
	}

	/* 寫入 Payload */
	if (!payload.empty())
	{
		buffer.insert(buffer.end(), payload.begin(), payload.end());
	}

	namespace VFS = NN::Runtime::VFS;
	return VFS::VFSService::WriteBufferToFile(path, buffer.data(), buffer.size());
}

} // namespace NN::Runtime::Asset
