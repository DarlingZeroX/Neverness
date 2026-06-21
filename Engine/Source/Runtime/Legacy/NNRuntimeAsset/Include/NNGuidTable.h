#pragma once

/**
 * @file NNGuidTable.h
 * @brief GUID ↔ VirtualPath 双向映射表。
 */

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

#include "../NNRuntimeAssetExport.h"
#include "NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::Asset
{

/**
 * @brief GUID ↔ VirtualPath 双向索引。
 */
class NN_ASSET_API NNGuidTable
{
public:
	/** @brief 註冊映射。若 GUID 或路徑已存在，覆蓋舊映射。 */
	void Register(const std::string& path, NNGuid guid);

	/** @brief 依 GUID 移除。回傳舊路徑（空字串表示不存在）。 */
	std::string UnregisterByGuid(NNGuid guid);

	/** @brief 依路徑移除。回傳舊 GUID（全零表示不存在）。 */
	NNGuid UnregisterByPath(const std::string& path);

	/** @brief GUID → 路徑。回傳 false 表示不存在。 */
	bool ResolvePath(NNGuid guid, std::string& outPath) const;

	/** @brief 路徑 → GUID。回傳 false 表示不存在。 */
	bool ResolveGuid(const std::string& path, NNGuid& outGuid) const;

	/** @brief 是否存在此 GUID。 */
	bool ContainsGuid(NNGuid guid) const;

	/** @brief 註冊的資產總數。 */
	std::uint32_t GetCount() const;

	/** @brief 清除所有映射。 */
	void Clear();

private:
	/* GUID.low → 路徑 */
	std::unordered_map<std::uint64_t, std::string> guidToPath_;
	/* 路徑 → GUID */
	std::unordered_map<std::string, NNGuid>        pathToGuid_;
};

} // namespace NN::Runtime::Asset
