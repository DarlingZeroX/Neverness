#include "NNStreamingManager.h"

#include <cstring>
#include <fstream>

namespace NN::Runtime::Asset
{

NNStreamingManager::NNStreamingManager() noexcept = default;

NNStreamingManager::~NNStreamingManager()
{
	Stop();
}

/* ======================== 啟動/停止 ======================== */

void NNStreamingManager::Start(std::uint32_t ioThreadCount, std::uint32_t decodeThreadCount)
{
	if (running_.load(std::memory_order_acquire))
		return;

	running_.store(true, std::memory_order_release);

	/* 啟動 IO 執行緒 */
	for (std::uint32_t i = 0; i < ioThreadCount; ++i)
	{
		ioThreads_.emplace_back(&NNStreamingManager::IoThreadFunc, this);
	}

	/* 啟動解碼執行緒 */
	for (std::uint32_t i = 0; i < decodeThreadCount; ++i)
	{
		decodeThreads_.emplace_back(&NNStreamingManager::DecodeThreadFunc, this);
	}
}

void NNStreamingManager::Stop()
{
	if (!running_.load(std::memory_order_acquire))
		return;

	running_.store(false, std::memory_order_release);
	requestCv_.notify_all();
	decodeCv_.notify_all();

	for (auto& t : ioThreads_)
	{
		if (t.joinable())
			t.join();
	}
	ioThreads_.clear();

	for (auto& t : decodeThreads_)
	{
		if (t.joinable())
			t.join();
	}
	decodeThreads_.clear();

	/* 清空所有佇列 */
	{
		std::lock_guard<std::mutex> lock(requestMutex_);
		while (!requestQueue_.empty())
			requestQueue_.pop();
	}
	{
		std::lock_guard<std::mutex> lock(decodeMutex_);
		while (!decodeQueue_.empty())
			decodeQueue_.pop();
	}
	{
		std::lock_guard<std::mutex> lock(completionMutex_);
		while (!completionQueue_.empty())
			completionQueue_.pop();
	}
}

/* ======================== 請求管理 ======================== */

void NNStreamingManager::SubmitRequest(const NNStreamingRequest& request)
{
	{
		std::lock_guard<std::mutex> lock(requestMutex_);
		requestQueue_.push(request);
	}
	requestCv_.notify_one();
}

void NNStreamingManager::CancelRequest(NNGuid guid)
{
	/* 標記請求佇列中的匹配項為取消 */
	std::lock_guard<std::mutex> lock(requestMutex_);

	/* 無法直接遍歷 priority_queue，使用臨時容器 */
	std::vector<NNStreamingRequest> temp;
	while (!requestQueue_.empty())
	{
		auto req = requestQueue_.top();
		requestQueue_.pop();
		if (req.guid.high == guid.high && req.guid.low == guid.low)
			req.cancelled = true;
		temp.push_back(req);
	}
	for (auto& r : temp)
		requestQueue_.push(r);
}

std::uint32_t NNStreamingManager::GetPendingRequestCount() const
{
	std::lock_guard<std::mutex> lock(requestMutex_);
	return static_cast<std::uint32_t>(requestQueue_.size());
}

/* ======================== 完成佇列 ======================== */

std::uint32_t NNStreamingManager::GetCompletedCount() const
{
	std::lock_guard<std::mutex> lock(completionMutex_);
	return static_cast<std::uint32_t>(completionQueue_.size());
}

bool NNStreamingManager::PopCompleted(NNAsyncIoResult& outResult)
{
	std::lock_guard<std::mutex> lock(completionMutex_);
	if (completionQueue_.empty())
		return false;
	outResult = std::move(completionQueue_.front());
	completionQueue_.pop();
	return true;
}

/* ======================== IO 執行緒 ======================== */

void NNStreamingManager::IoThreadFunc()
{
	while (running_.load(std::memory_order_acquire))
	{
		NNStreamingRequest request;
		{
			std::unique_lock<std::mutex> lock(requestMutex_);
			requestCv_.wait(lock, [this] {
				return !requestQueue_.empty() || !running_.load(std::memory_order_acquire);
			});

			if (!running_.load(std::memory_order_acquire))
				break;
			if (requestQueue_.empty())
				break;

			request = requestQueue_.top();
			requestQueue_.pop();
		}

		/* 檢查是否已取消 */
		if (request.cancelled)
			continue;

		/* 構建 IO 結果 */
		NNAsyncIoResult result;
		result.guid = request.guid;
		result.typeId = request.typeId;
		result.callback = request.callback;
		result.callbackUserData = request.callbackUserData;
		result.success = false;

		/*
		 * 解析 .nnasset 路徑：
		 * Library/Imported/XX/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.nnasset
		 *
		 * GUID → hex 路徑轉換（與 C++ NNAssetManager::ResolveAssetPath 對齊）
		 */
		char hexBuf[33];
		std::snprintf(hexBuf, sizeof(hexBuf), "%016llx%016llx",
		              static_cast<unsigned long long>(request.guid.high),
		              static_cast<unsigned long long>(request.guid.low));

		/* 嘗試多個搜索路徑 */
		const std::string hexStr(hexBuf);
		const std::string prefix = hexStr.substr(0, 2);
		const std::string fileName = hexStr + ".nnasset";

		std::vector<std::string> searchPaths = {
			"Library/Imported/" + prefix + "/" + fileName,
			"Assets/Imported/" + prefix + "/" + fileName,
			fileName
		};

		/* 非同步讀取檔案（同步 fallback，實際可使用 Windows Overlapped IO） */
		for (const auto& path : searchPaths)
		{
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (!file.is_open())
				continue;

			const auto fileSize = static_cast<std::size_t>(file.tellg());
			file.seekg(0);

			if (fileSize < 64) /* 至少要有 NNAssetHeader */
				continue;

			result.data.resize(fileSize);
			file.read(reinterpret_cast<char*>(result.data.data()),
			          static_cast<std::streamsize>(fileSize));

			if (file)
			{
				result.success = true;
				break;
			}
		}

		/* 送入解碼佇列 */
		{
			std::lock_guard<std::mutex> lock(decodeMutex_);
			decodeQueue_.push(std::move(result));
		}
		decodeCv_.notify_one();
	}
}

/* ======================== 解碼執行緒 ======================== */

void NNStreamingManager::DecodeThreadFunc()
{
	while (running_.load(std::memory_order_acquire))
	{
		NNAsyncIoResult result;
		{
			std::unique_lock<std::mutex> lock(decodeMutex_);
			decodeCv_.wait(lock, [this] {
				return !decodeQueue_.empty() || !running_.load(std::memory_order_acquire);
			});

			if (!running_.load(std::memory_order_acquire))
				break;
			if (decodeQueue_.empty())
				break;

			result = std::move(decodeQueue_.front());
			decodeQueue_.pop();
		}

		/* 解碼處理：
		 *   1. 驗證 .nnasset header（magic, version）
		 *   2. 解壓縮 blob（如有 compressedSize > 0）
		 *   3. 資料格式轉換（紋理格式、網格優化等）
		 *
		 * 當前實作：驗證 header + 直接傳遞資料
		 * 完整解壓/格式轉換將在需要時實現
		 */
		if (result.success && result.data.size() >= 64)
		{
			/* 驗證 magic = "NNAS" */
			std::uint32_t magic = 0;
			std::memcpy(&magic, result.data.data(), 4);
			if (magic != 0x4E4E4153u)
			{
				result.success = false;
			}
			else
			{
				/* 驗證 version = 1 */
				std::uint32_t version = 0;
				std::memcpy(&version, result.data.data() + 4, 4);
				if (version != 1u)
					result.success = false;
			}
		}

		/* TODO: 解壓縮支援
		 * 檢查 header.flags & NN_ASSET_FLAG_COMPRESSED
		 * 如果 compressed，對每個 blob 的 CompressedData 進行 Zstd/LZ4 解壓
		 * 將解壓後資料覆蓋回 data 區域
		 */

		/* TODO: 格式轉換
		 * 根據 typeId 執行特定轉換：
		 *   Texture2D → GPU 紋理格式（BC1/BC7/RGBA8）
		 *   Mesh → 頂點/索引緩衝優化
		 *   AudioClip → PCM 樣本提取
		 */

		/* 送入完成佇列（供主線程 Tick() 消費） */
		{
			std::lock_guard<std::mutex> lock(completionMutex_);
			completionQueue_.push(std::move(result));
		}
	}
}

} // namespace NN::Runtime::Asset
