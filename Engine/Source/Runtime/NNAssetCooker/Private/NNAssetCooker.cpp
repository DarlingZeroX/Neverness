#include "NNAssetCooker.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace NN::Runtime::Asset
{

NNCookResult NNAssetCooker::Cook(const NNCookManifest& manifest,
                                  NNCookProgressCallback progress)
{
	NNCookResult result;
	const auto startTime = std::chrono::steady_clock::now();

	const auto& groups = manifest.GetGroups();
	const auto& assets = manifest.GetAssets();

	result.totalAssets = static_cast<std::uint32_t>(assets.size());

	if (groups.empty())
	{
		/* 无分组：将所有资产打包为单个 .nnpack */
		NNPackBuilder builder;
		builder.SetOutputPath(manifest.GetOutputRoot() + "/default.nnpack");
		builder.SetPackageName("default");

		std::uint32_t idx = 0;
		for (const auto& entry : assets)
		{
			if (!entry.includeInBuild)
				continue;

			if (progress)
				progress(idx, result.totalAssets, "Cooking asset");

			/* 读取 .nnasset 文件 */
			std::ifstream file(entry.sourcePath, std::ios::binary | std::ios::ate);
			if (!file.is_open())
			{
				result.failedAssets++;
				idx++;
				continue;
			}

			auto fileSize = static_cast<std::size_t>(file.tellg());
			file.seekg(0);

			std::vector<std::uint8_t> data(fileSize);
			file.read(reinterpret_cast<char*>(data.data()),
			          static_cast<std::streamsize>(fileSize));

			builder.AddAsset(entry.guid, entry.typeId, data);
			result.cookedAssets++;
			idx++;
		}

		if (builder.Build())
			result.generatedPacks++;
		else
		{
			result.success = false;
			result.errorMessage = "Failed to build default pack";
		}
	}
	else
	{
		/* 按分组构建 */
		std::uint32_t groupIdx = 0;
		for (const auto& group : groups)
		{
			if (progress)
				progress(groupIdx, static_cast<std::uint32_t>(groups.size()),
				         ("Cooking group: " + group.name).c_str());

			if (CookGroup(group, manifest, progress, groupIdx,
			              static_cast<std::uint32_t>(groups.size())))
			{
				result.generatedPacks++;
			}
			else
			{
				result.failedAssets++;
			}
			groupIdx++;
		}
	}

	result.success = (result.failedAssets == 0);

	const auto endTime = std::chrono::steady_clock::now();
	result.elapsedSeconds = std::chrono::duration<double>(endTime - startTime).count();

	return result;
}

bool NNAssetCooker::CookGroup(const NNCookGroup& group,
                               const NNCookManifest& manifest,
                               NNCookProgressCallback progress,
                               std::uint32_t groupIndex,
                               std::uint32_t totalGroups)
{
	NNPackBuilder builder;

	std::string outputPath = group.outputPath;
	if (outputPath.empty())
		outputPath = manifest.GetOutputRoot() + "/" + group.name + ".nnpack";

	builder.SetOutputPath(outputPath);
	builder.SetPackageName(group.name);

	const auto& assets = manifest.GetAssets();
	std::uint32_t cookedCount = 0;

	for (const auto& entry : assets)
	{
		if (entry.groupIndex != groupIndex)
			continue;
		if (!entry.includeInBuild)
			continue;

		/* 读取 .nnasset 文件 */
		std::string fullPath = entry.sourcePath;

		/* 如果是相对路径，加上 Library 根目录 */
		if (!std::filesystem::path(fullPath).is_absolute() && !manifest.GetLibraryRoot().empty())
			fullPath = manifest.GetLibraryRoot() + "/" + fullPath;

		std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
		if (!file.is_open())
			continue;

		auto fileSize = static_cast<std::size_t>(file.tellg());
		file.seekg(0);

		std::vector<std::uint8_t> data(fileSize);
		file.read(reinterpret_cast<char*>(data.data()),
		          static_cast<std::streamsize>(fileSize));

		builder.AddAsset(entry.guid, entry.typeId, data);
		cookedCount++;

		if (progress)
			progress(cookedCount, static_cast<std::uint32_t>(assets.size()),
			         ("  Asset: " + std::to_string(cookedCount)).c_str());
	}

	return builder.Build();
}

} // namespace NN::Runtime::Asset
