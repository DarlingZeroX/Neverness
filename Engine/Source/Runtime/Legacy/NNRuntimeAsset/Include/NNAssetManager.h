#pragma once

/**
 * @file NNAssetManager.h
 * @brief Runtime 資產管理器（核心）。
 *
 * 職責：
 *   - GUID → Handle 映射
 *   - 同步/異步資產載入
 *   - .nnasset 檔案解析
 *   - 引用計數管理
 *   - 快取與驅逐
 *   - 包掛載
 *   - Hot Reload 支援
 *
 * 設計原則：
 *   - Runtime 不知道 .meta / importer / AssetDatabase
 *   - async-first
 *   - cache-friendly
 */

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "../NNRuntimeAssetExport.h"
#include "GuidHashMap.h"

#include "NNAssetCache.h"
#include "NNAssetFormat.h"
#include "NNAssetHandle.h"
#include "NNAssetTypes.h"
#include "NNStreamingManager.h"
#include "NNNativeEngineAPI/Include/EngineTypes.h"
#include "NNNativeEngineAPI/Include/EngineHandles.h"
#include "NNNativeEngineAPI/Include/AssetManagerAPI.h"

namespace NN::Runtime::Asset
{

/**
 * @brief 資產條目（內部資料）。
 */
struct NNAssetEntry
{
	NNGuid               guid{};
	std::uint64_t        typeId{0};
	std::uint64_t        handle{0};       /* HandleTable 中的原始值 */

	std::atomic<std::uint32_t> refCount{0};

	enum class State : std::uint8_t
	{
		Unloaded,
		Loading,
		Loaded,
		Failed,
		Streaming
	};
	std::atomic<State> state{State::Unloaded};

	/* 依賴 */
	std::vector<NNGuid>  dependencies;
	std::atomic<std::uint32_t> loadedDependencyCount{0};

	/* 資料 */
	std::vector<std::uint8_t> data;           /* 完整 .nnasset 資料 */
	std::vector<NNBlobDescriptor> blobs;      /* blob 描述符快取 */
	std::uint64_t        payloadOffset{0};    /* payload 區段在 data 中的偏移 */

	/* IO */
	std::string          sourcePath;          /* .nnasset 路徑（或包內路徑） */
	bool                 fromPackage{false};
	std::uint32_t        packageIndex{0};

	/* Hot Reload */
	std::atomic<bool>    needsReload{false};
};

/**
 * @brief Runtime 資產管理器。
 */
class NN_ASSET_API NNAssetManager
{
public:
	static NNAssetManager& Instance() noexcept;

	/* === 初始化/關閉 === */
	bool Initialize(const std::string& assetRoot = "");
	void Shutdown();

	/* === 每幀更新 === */
	void Tick();

	/* === 同步載入 === */
	NNAssetHandleT<void> LoadAssetSync(const NNGuid& guid, std::uint64_t typeId = 0);

	/// 仅通过 guid.low 同步加载资产（用于 TextureAsset 仅存 guid.low 的场景）
	NNAssetHandleT<void> LoadAssetByGuidLow(std::uint64_t guidLow, std::uint64_t typeId = 0);

	/* === 異步載入 === */
	NNAsyncWaitHandle LoadAssetAsync(
		const NNGuid& guid,
		std::uint64_t typeId,
		NNLoadPriority priority,
		NNAssetLoadCompletedCallback callback = nullptr,
		void* callbackUserData = nullptr);

	/* === 卸載 === */
	void UnloadAsset(std::uint64_t rawHandle);
	void UnloadAssetByGuid(const NNGuid& guid);

	/* === 查詢 === */
	bool IsLoaded(std::uint64_t rawHandle) const;
	bool IsLoading(std::uint64_t rawHandle) const;
	NNAssetHandleT<void> GetLoadedAsset(const NNGuid& guid) const;
	NNGuid GetGuidByHandle(std::uint64_t rawHandle) const;

	/* === 引用計數 === */
	void AddRef(std::uint64_t rawHandle);
	void ReleaseRef(std::uint64_t rawHandle);
	std::uint32_t GetRefCount(std::uint64_t rawHandle) const;

	/* === 資料存取 === */
	const void* GetAssetData(std::uint64_t rawHandle) const;
	std::uint64_t GetAssetDataSize(std::uint64_t rawHandle) const;
	std::uint32_t GetBlobCount(std::uint64_t rawHandle) const;
	const void* GetBlobData(std::uint64_t rawHandle, std::uint32_t index) const;
	std::uint64_t GetBlobSize(std::uint64_t rawHandle, std::uint32_t index) const;
	const NNBlobDescriptor* GetBlobDesc(std::uint64_t rawHandle, std::uint32_t index) const;
	const NNBlobDescriptor* GetBlobByType(std::uint64_t rawHandle, std::uint32_t blobType, const void** outData = nullptr) const;

	/* === 包管理 === */
	bool MountPackage(const std::string& packPath);
	void UnmountPackage(const std::string& packPath);
	bool IsAssetInPackage(const NNGuid& guid) const;

	/* === Hot Reload === */
	void MarkForReload(const NNGuid& guid);
	void ReloadMarkedAssets();

	/* === 統計 === */
	std::uint64_t GetLoadedAssetCount() const;
	std::uint64_t GetTotalMemoryUsage() const;

	/* === 取得子系統 === */
	NNHandleTable&       GetHandleTable() noexcept { return handleTable_; }
	NNAssetCache&        GetCache() noexcept { return cache_; }
	NNStreamingManager&  GetStreaming() noexcept { return streaming_; }

private:
	NNAssetManager() = default;

	/* 載入實作 */
	NNAssetHandleT<void> LoadAssetInternal(const NNGuid& guid, std::uint64_t typeId);
	bool ReadNnAsset(const std::string& path, NNAssetEntry& entry);
	std::string ResolveAssetPath(const NNGuid& guid) const;
	static bool GuidIsZero(const NNGuid& g) noexcept;

	/* 子系統 */
	NNHandleTable       handleTable_;
	NNAssetCache        cache_;
	NNStreamingManager  streaming_;

	/* 索引 */
	mutable std::mutex  mutex_;
	GuidHashMap<std::shared_ptr<NNAssetEntry>> guidToEntry_;   /* guid.low → entry */
	GuidHashMap<std::shared_ptr<NNAssetEntry>> handleToEntry_; /* handle → entry */

	/* 包 */
	std::vector<std::string> mountedPackages_;

	/* 路徑 */
	std::string assetRoot_;

	/* 狀態 */
	bool initialized_{false};
};

} // namespace NN::Runtime::Asset
