#include "NNAssetManager.h"

#include "NNPackManager.h"
#include <NNCore/Interface/HLog.h>
#include <NNRuntimeVFS/Include/VFSService.h>

#include <cstring>

namespace NN::Runtime::Asset
{

/* ======================== 單例 ======================== */

NNAssetManager& NNAssetManager::Instance() noexcept
{
	static NNAssetManager instance;
	return instance;
}

/* ======================== 初始化/關閉 ======================== */

bool NNAssetManager::Initialize(const std::string& assetRoot)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (initialized_)
		return true;

	assetRoot_ = assetRoot;
	cache_.SetMemoryBudget(512ull * 1024 * 1024); /* 預設 512MB */

	/* 註冊預設資產型別 */
	auto& registry = NNAssetTypeRegistry::Instance();
	registry.RegisterType("Texture2D",   NN_TYPE_ID_TEXTURE_2D);
	registry.RegisterType("Mesh",        NN_TYPE_ID_MESH);
	registry.RegisterType("AudioClip",   NN_TYPE_ID_AUDIO_CLIP);
	registry.RegisterType("Material",    NN_TYPE_ID_MATERIAL);
	registry.RegisterType("Shader",      NN_TYPE_ID_SHADER);
	registry.RegisterType("Scene",       NN_TYPE_ID_SCENE);
	registry.RegisterType("Prefab",      NN_TYPE_ID_PREFAB);
	registry.RegisterType("Animation",   NN_TYPE_ID_ANIMATION);
	registry.RegisterType("LuaScript",   NN_TYPE_ID_LUA_SCRIPT);

	streaming_.Start(2, 2);

	initialized_ = true;
	return true;
}

void NNAssetManager::Shutdown()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (!initialized_)
		return;

	streaming_.Stop();

	/* 釋放所有條目 */
	guidToEntry_.Clear();
	handleToEntry_.Clear();
	mountedPackages_.clear();

	initialized_ = false;
}

void NNAssetManager::Tick()
{
	std::lock_guard<std::mutex> lock(mutex_);

	if (!initialized_)
		return;

	/* 處理 Streaming 完成佇列 */
	NNAsyncIoResult result;
	while (streaming_.PopCompleted(result))
	{
		if (result.guid.high == 0 && result.guid.low == 0)
			continue;

		/* 查找或建立 AssetEntry */
		auto it = guidToEntry_.Find(result.guid.low);
		std::shared_ptr<NNAssetEntry> entry;

		if (it != nullptr)
		{
			entry = (*it);
		}
		else
		{
			entry = std::make_shared<NNAssetEntry>();
			entry->guid = result.guid;
			entry->typeId = result.typeId;
		}

		if (result.success && !result.data.empty())
		{
			/* 設置資料 */
			entry->data = std::move(result.data);

			/* 解析 Header */
			if (entry->data.size() >= sizeof(NNAssetHeader))
			{
				NNAssetHeader header{};
				std::memcpy(&header, entry->data.data(), sizeof(NNAssetHeader));

				if (NNAssetHeaderIsValid(&header) != 0)
				{
					entry->typeId = header.typeId;

					/* 解析依賴 */
					entry->dependencies.clear();
					if (header.dependencyCount > 0
					    && header.dependencyOffset + header.dependencyCount * sizeof(NNGuid) <= entry->data.size())
					{
						const auto* deps = reinterpret_cast<const NNGuid*>(
							entry->data.data() + header.dependencyOffset);
						entry->dependencies.assign(deps, deps + header.dependencyCount);
					}

					/* 解析 Blob 描述符 */
					entry->blobs.clear();
					if (header.blobCount > 0
					    && header.blobTableOffset + header.blobCount * sizeof(NNBlobDescriptor) <= entry->data.size())
					{
						const auto* blobs = reinterpret_cast<const NNBlobDescriptor*>(
							entry->data.data() + header.blobTableOffset);
						entry->blobs.assign(blobs, blobs + header.blobCount);
					}
				}
			}

			/* 分配 Handle（如果尚未分配） */
			if (entry->handle == 0)
			{
				const std::uint64_t rawHandle = handleTable_.Allocate(entry.get(), entry->typeId);
				entry->handle = rawHandle;
			}

			entry->state.store(NNAssetEntry::State::Loaded, std::memory_order_release);

			/* 更新索引 */
			guidToEntry_.Insert(result.guid.low, entry);
			handleToEntry_.Insert(entry->handle, entry);

			/* 更新快取 */
			cache_.Touch(result.guid, entry->data.size(), entry->handle);
		}
		else
		{
			entry->state.store(NNAssetEntry::State::Failed, std::memory_order_release);
		}

		/* 觸發回調 */
		if (result.callback)
		{
			const auto handle = static_cast<NNAssetHandle>(entry->handle);
			const auto error = result.success ? 0 : 1;
			result.callback(handle, error, result.callbackUserData);
		}
	}

	/* Hot Reload：檢查標記的資產 */
	for (auto& [key, entry] : guidToEntry_)
	{
		if (!entry->needsReload.load(std::memory_order_acquire))
			continue;

		entry->needsReload.store(false, std::memory_order_release);

		if (ReadNnAsset(entry->sourcePath, *entry))
		{
			entry->state.store(NNAssetEntry::State::Loaded, std::memory_order_release);
		}
		else
		{
			entry->state.store(NNAssetEntry::State::Failed, std::memory_order_release);
		}
	}
}

/* ======================== 同步載入 ======================== */

NNAssetHandleT<void> NNAssetManager::LoadAssetSync(const NNGuid& guid, std::uint64_t typeId)
{
	std::lock_guard<std::mutex> lock(mutex_);
	return LoadAssetInternal(guid, typeId);
}

NNAssetHandleT<void> NNAssetManager::LoadAssetInternal(const NNGuid& guid, std::uint64_t typeId)
{
	if (GuidIsZero(guid))
	{
		H_LOG_WARN("[AssetManager] LoadAssetInternal: GUID 為零，跳過載入");
		return NNAssetHandleT<void>();
	}

	H_LOG_INFO("[AssetManager] LoadAssetInternal: 開始載入資產 GUID=%016llx%016llx typeId=%llu",
	           static_cast<unsigned long long>(guid.high),
	           static_cast<unsigned long long>(guid.low),
	           static_cast<unsigned long long>(typeId));

	/* 檢查是否已載入 */
	auto itGuid = guidToEntry_.Find(guid.low);
	if (itGuid != nullptr && (*itGuid)->state.load(std::memory_order_acquire) == NNAssetEntry::State::Loaded)
	{
		auto entry = (*itGuid);
		handleTable_.AddRef(entry->handle);
		H_LOG_INFO("[AssetManager] LoadAssetInternal: 資產已載入，增加引用 handle=%llu refCount=%u",
		           static_cast<unsigned long long>(entry->handle),
		           entry->refCount.load(std::memory_order_relaxed));
		return NNAssetHandleT<void>(entry->handle);
	}

	/* 解析路徑 */
	std::string path = ResolveAssetPath(guid);
	if (path.empty())
	{
		H_LOG_ERROR("[AssetManager] LoadAssetInternal: 無法解析資產路徑 GUID=%016llx%016llx",
		            static_cast<unsigned long long>(guid.high),
		            static_cast<unsigned long long>(guid.low));
		return NNAssetHandleT<void>();
	}

	H_LOG_INFO("[AssetManager] LoadAssetInternal: 解析路徑為 %s", path.c_str());

	/* 建立條目 */
	auto entry = std::make_shared<NNAssetEntry>();
	entry->guid = guid;
	entry->typeId = typeId;
	entry->sourcePath = path;
	entry->state.store(NNAssetEntry::State::Loading, std::memory_order_release);

	/* 讀取 .nnasset */
	if (!ReadNnAsset(path, *entry))
	{
		entry->state.store(NNAssetEntry::State::Failed, std::memory_order_release);
		H_LOG_ERROR("[AssetManager] LoadAssetInternal: 讀取 .nnasset 失敗 path=%s", path.c_str());
		return NNAssetHandleT<void>();
	}

	/* 分配 Handle */
	const std::uint64_t rawHandle = handleTable_.Allocate(entry.get(), typeId);
	entry->handle = rawHandle;
	entry->state.store(NNAssetEntry::State::Loaded, std::memory_order_release);

	/* 更新索引 */
	guidToEntry_.Insert(guid.low, entry);
	handleToEntry_.Insert(rawHandle, entry);

	/* 更新快取 */
	cache_.Touch(guid, entry->data.size(), rawHandle);

	H_LOG_INFO("[AssetManager] LoadAssetInternal: 載入成功 handle=%llu data=%zu bytes deps=%zu blobs=%zu",
	           static_cast<unsigned long long>(rawHandle),
	           entry->data.size(),
	           entry->dependencies.size(),
	           entry->blobs.size());

	return NNAssetHandleT<void>(rawHandle);
}

/* ======================== 異步載入 ======================== */

NNAsyncWaitHandle NNAssetManager::LoadAssetAsync(
	const NNGuid& guid,
	std::uint64_t typeId,
	NNLoadPriority priority,
	NNAssetLoadCompletedCallback callback,
	void* callbackUserData)
{
	/* 檢查是否已載入 */
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = guidToEntry_.Find(guid.low);
		if (it != nullptr && (*it)->state.load(std::memory_order_acquire) == NNAssetEntry::State::Loaded)
		{
			if (callback)
				callback(static_cast<NNAssetHandle>((*it)->handle), 0, callbackUserData);
			return 0; /* 已載入，無需等待 */
		}
	}

	/* 提交串流請求 */
	NNStreamingRequest req{};
	req.guid = guid;
	req.typeId = typeId;
	req.priority = priority;
	req.callback = callback;
	req.callbackUserData = callbackUserData;
	streaming_.SubmitRequest(req);

	return 1; /* 非零表示進行中 */
}

/* ======================== 卸載 ======================== */

void NNAssetManager::UnloadAsset(std::uint64_t rawHandle)
{
	std::lock_guard<std::mutex> lock(mutex_);

	auto it = handleToEntry_.Find(rawHandle);
	if (it == nullptr)
		return;

	auto entry = (*it);
	const std::uint32_t prev = entry->refCount.fetch_sub(1, std::memory_order_acq_rel);
	if (prev > 1)
		return; /* 引用計數未降至 0 */

	/* 引用計數歸零，釋放 */
	cache_.Remove(entry->guid);
	guidToEntry_.Erase(entry->guid.low);
	handleToEntry_.Erase(rawHandle);
	handleTable_.Free(rawHandle);
}

void NNAssetManager::UnloadAssetByGuid(const NNGuid& guid)
{
	std::lock_guard<std::mutex> lock(mutex_);

	auto it = guidToEntry_.Find(guid.low);
	if (it == nullptr)
		return;

	auto entry = (*it);
	const std::uint32_t prev = entry->refCount.fetch_sub(1, std::memory_order_acq_rel);
	if (prev > 1)
		return;

	cache_.Remove(guid);
	handleToEntry_.Erase(entry->handle);
	guidToEntry_.Erase(guid.low);
	handleTable_.Free(entry->handle);
}

/* ======================== 查詢 ======================== */

bool NNAssetManager::IsLoaded(std::uint64_t rawHandle) const
{
	auto* data = handleTable_.Resolve(rawHandle);
	return data != nullptr;
}

bool NNAssetManager::IsLoading(std::uint64_t rawHandle) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = handleToEntry_.Find(rawHandle);
	if (it == nullptr)
		return false;
	return (*it)->state.load(std::memory_order_acquire) == NNAssetEntry::State::Loading;
}

NNAssetHandleT<void> NNAssetManager::GetLoadedAsset(const NNGuid& guid) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = guidToEntry_.Find(guid.low);
	if (it == nullptr)
		return NNAssetHandleT<void>();
	if ((*it)->state.load(std::memory_order_acquire) != NNAssetEntry::State::Loaded)
		return NNAssetHandleT<void>();
	return NNAssetHandleT<void>((*it)->handle);
}

NNGuid NNAssetManager::GetGuidByHandle(std::uint64_t rawHandle) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = handleToEntry_.Find(rawHandle);
	if (it == nullptr)
	{
		NNGuid z{};
		return z;
	}
	return (*it)->guid;
}

/* ======================== 引用計數 ======================== */

void NNAssetManager::AddRef(std::uint64_t rawHandle)
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = handleToEntry_.Find(rawHandle);
	if (it != nullptr)
	{
		(*it)->refCount.fetch_add(1, std::memory_order_relaxed);
		handleTable_.AddRef(rawHandle);
	}
}

void NNAssetManager::ReleaseRef(std::uint64_t rawHandle)
{
	UnloadAsset(rawHandle);
}

std::uint32_t NNAssetManager::GetRefCount(std::uint64_t rawHandle) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = handleToEntry_.Find(rawHandle);
	if (it == nullptr)
		return 0;
	return (*it)->refCount.load(std::memory_order_relaxed);
}

/* ======================== 資料存取 ======================== */

const void* NNAssetManager::GetAssetData(std::uint64_t rawHandle) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = handleToEntry_.Find(rawHandle);
	if (it == nullptr || (*it)->data.empty())
		return nullptr;
	return (*it)->data.data();
}

std::uint64_t NNAssetManager::GetAssetDataSize(std::uint64_t rawHandle) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = handleToEntry_.Find(rawHandle);
	if (it == nullptr)
		return 0;
	return (*it)->data.size();
}

std::uint32_t NNAssetManager::GetBlobCount(std::uint64_t rawHandle) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = handleToEntry_.Find(rawHandle);
	if (it == nullptr)
		return 0;
	return static_cast<std::uint32_t>((*it)->blobs.size());
}

const void* NNAssetManager::GetBlobData(std::uint64_t rawHandle, std::uint32_t index) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = handleToEntry_.Find(rawHandle);
	if (it == nullptr || index >= (*it)->blobs.size())
		return nullptr;

	const auto& blob = (*it)->blobs[index];
	if ((*it)->data.empty())
		return nullptr;

	return (*it)->data.data() + (*it)->payloadOffset + blob.offset;
}

std::uint64_t NNAssetManager::GetBlobSize(std::uint64_t rawHandle, std::uint32_t index) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = handleToEntry_.Find(rawHandle);
	if (it == nullptr || index >= (*it)->blobs.size())
		return 0;
	return (*it)->blobs[index].size;
}

const NNBlobDescriptor* NNAssetManager::GetBlobByType(std::uint64_t rawHandle, std::uint32_t blobType, const void** outData) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = handleToEntry_.Find(rawHandle);
	if (it == nullptr)
		return nullptr;
	for (const auto& b : (*it)->blobs)
	{
		if (b.blobType == blobType)
		{
			/* blob offset 是相对于 payloadOffset 的，需要加上 payloadOffset */
			auto absOffset = (*it)->payloadOffset + b.offset;
			if (absOffset + b.size > (*it)->data.size())
			{
				H_LOG_ERROR("[AssetManager] GetBlobByType: blob type=%u payloadOffset=%llu relOffset=%llu size=%llu 越界 (data size=%llu)",
					blobType, (*it)->payloadOffset, b.offset, b.size, (unsigned long long)(*it)->data.size());
				return nullptr;
			}
			if (outData)
				*outData = (*it)->data.data() + absOffset;
			return &b;
		}
	}
	return nullptr;
}

/* ======================== 包管理 ======================== */

bool NNAssetManager::MountPackage(const std::string& packPath)
{
	auto& pm = NNPackManager::Instance();
	if (!pm.MountPackage(packPath))
		return false;

	std::lock_guard<std::mutex> lock(mutex_);
	mountedPackages_.push_back(packPath);
	return true;
}

void NNAssetManager::UnmountPackage(const std::string& packPath)
{
	auto& pm = NNPackManager::Instance();
	pm.UnmountPackage(packPath);

	std::lock_guard<std::mutex> lock(mutex_);
	auto it = std::find(mountedPackages_.begin(), mountedPackages_.end(), packPath);
	if (it != mountedPackages_.end())
		mountedPackages_.erase(it);
}

bool NNAssetManager::IsAssetInPackage(const NNGuid& guid) const
{
	return NNPackManager::Instance().IsAssetInPackage(guid);
}

/* ======================== Hot Reload ======================== */

void NNAssetManager::MarkForReload(const NNGuid& guid)
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = guidToEntry_.Find(guid.low);
	if (it != nullptr)
		(*it)->needsReload.store(true, std::memory_order_release);
}

void NNAssetManager::ReloadMarkedAssets()
{
	std::lock_guard<std::mutex> lock(mutex_);

	for (auto& [key, entry] : guidToEntry_)
	{
		if (!entry->needsReload.load(std::memory_order_acquire))
			continue;

		entry->needsReload.store(false, std::memory_order_release);

		/* 重新讀取 .nnasset */
		if (ReadNnAsset(entry->sourcePath, *entry))
		{
			entry->state.store(NNAssetEntry::State::Loaded, std::memory_order_release);
		}
		else
		{
			entry->state.store(NNAssetEntry::State::Failed, std::memory_order_release);
		}
	}
}

/* ======================== 統計 ======================== */

std::uint64_t NNAssetManager::GetLoadedAssetCount() const
{
	return handleTable_.GetAllocatedCount();
}

std::uint64_t NNAssetManager::GetTotalMemoryUsage() const
{
	return cache_.GetCurrentUsage();
}

/* ======================== 內部工具 ======================== */

bool NNAssetManager::GuidIsZero(const NNGuid& g) noexcept
{
	return g.high == 0 && g.low == 0;
}

bool NNAssetManager::ReadNnAsset(const std::string& path, NNAssetEntry& entry)
{
	namespace VFS = NN::Runtime::VFS;

	auto file = VFS::VFSService::GetInstance()->OpenFile(
		VFS::FileInfo(path), VFS::IFile::FileMode::Read);
	if (!file || !file->IsOpened())
		return false;

	const auto fileSize = static_cast<std::size_t>(file->Size());
	if (fileSize < sizeof(NNAssetHeader))
		return false;

	/* 讀取整個檔案到 entry.data */
	entry.data.resize(fileSize);
	auto bytesRead = file->Read(entry.data.data(), fileSize);
	file->Close();

	if (bytesRead < fileSize)
	{
		H_LOG_WARN("[AssetManager] ReadNnAsset: 请求 %llu 字节但只读取了 %llu 字节 path=%s",
			(unsigned long long)fileSize, (unsigned long long)bytesRead, path.c_str());
	}

	/* 尾部 hex dump（最后 32 字节） */
	if (fileSize >= 32)
	{
		auto* tail = entry.data.data() + fileSize - 32;
		H_LOG_INFO("[AssetManager] ReadNnAsset: 文件尾部 hex: "
			"%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x "
			"%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
			tail[0], tail[1], tail[2], tail[3], tail[4], tail[5], tail[6], tail[7],
			tail[8], tail[9], tail[10], tail[11], tail[12], tail[13], tail[14], tail[15],
			tail[16], tail[17], tail[18], tail[19], tail[20], tail[21], tail[22], tail[23],
			tail[24], tail[25], tail[26], tail[27], tail[28], tail[29], tail[30], tail[31]);
	}

	/* 解析 Header */
	NNAssetHeader header{};
	std::memcpy(&header, entry.data.data(), sizeof(NNAssetHeader));

	if (NNAssetHeaderIsValid(&header) == 0)
		return false;

	entry.typeId = header.typeId;
	entry.payloadOffset = header.payloadOffset;

	/* 解析依賴 */
	entry.dependencies.clear();
	if (header.dependencyCount > 0 && header.dependencyOffset + header.dependencyCount * sizeof(NNGuid) <= fileSize)
	{
		const auto* deps = reinterpret_cast<const NNGuid*>(entry.data.data() + header.dependencyOffset);
		entry.dependencies.assign(deps, deps + header.dependencyCount);
	}

	/* 解析 Blob 描述符 */
	entry.blobs.clear();
	if (header.blobCount > 0 && header.blobTableOffset + header.blobCount * sizeof(NNBlobDescriptor) <= fileSize)
	{
		const auto* blobs = reinterpret_cast<const NNBlobDescriptor*>(entry.data.data() + header.blobTableOffset);
		entry.blobs.assign(blobs, blobs + header.blobCount);
	}

	return true;
}

std::string NNAssetManager::ResolveAssetPath(const NNGuid& guid) const
{
	/* VFS 路徑：Library/Imported/XX/guid.nnasset
	 * 由 VFS 自動路由到對應的檔案系統
	 */
	char hexBuf[33];
	std::snprintf(hexBuf, sizeof(hexBuf), "%016llx%016llx",
	              static_cast<unsigned long long>(guid.high),
	              static_cast<unsigned long long>(guid.low));

	const std::string prefix(hexBuf, 2);
	const std::string fileName(hexBuf);

	return "Library/Imported/" + prefix + "/" + fileName + ".nnasset";
}

} // namespace NN::Runtime::Asset
