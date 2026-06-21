#pragma once

/**
 * @file NNStreamingManager.h
 * @brief 異步資產串流管理器。
 *
 * 架構：
 *   IO Thread Pool → 磁碟讀取（std::async / 同步 fallback）
 *   Decode Thread Pool → 解壓/格式轉換（CPU 密集）
 *   Completion Queue → 完成回調排隊（主線程 Tick() 消費）
 *   Request Queue → 優先級排序
 *
 * 設計原則：
 *   - async-first：所有載入預設非同步
 *   - priority-driven：優先級高的請求優先處理
 *   - completion-queue：回調統一由主線程 Tick() 觸發，避免多線程回調問題
 */

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "NNNativeEngineAPI/Include/EngineTypes.h"
#include "NNNativeEngineAPI/Include/AssetManagerAPI.h"

namespace NN::Runtime::Asset
{

/**
 * @brief 串流請求。
 */
struct NNStreamingRequest
{
	NNGuid             guid{};
	std::uint64_t      typeId{0};
	NNLoadPriority     priority{NN_LOAD_PRIORITY_NORMAL};
	NNAssetLoadCompletedCallback callback{nullptr};
	void*              callbackUserData{nullptr};
	float              distance{0.0f};    /* 距離相機的距離（用於排序） */
	std::uint32_t      targetMipLevel{0}; /* 目標 mip 層級（0 = 全部） */

	/* 取消標記（佇列已由 requestMutex_ 保護，不需要 atomic） */
	bool               cancelled{false};

	/* 優先級比較（值越小越優先） */
	bool operator<(const NNStreamingRequest& other) const
	{
		if (priority != other.priority)
			return static_cast<int>(priority) > static_cast<int>(other.priority);
		return distance > other.distance;
	}
};

/**
 * @brief 異步 IO 結果。
 */
struct NNAsyncIoResult
{
	NNGuid             guid{};
	std::uint64_t      typeId{0};
	NNAssetLoadCompletedCallback callback{nullptr};
	void*              callbackUserData{nullptr};

	std::vector<std::uint8_t> data;           /* 讀取的檔案資料 */
	bool              success{false};
	std::uint64_t     handle{0};              /* 由 AssetManager 在 Decode 階段分配 */
};

/**
 * @brief 異步串流管理器。
 */
class NNStreamingManager
{
public:
	NNStreamingManager() noexcept;
	~NNStreamingManager();

	NNStreamingManager(const NNStreamingManager&) = delete;
	NNStreamingManager& operator=(const NNStreamingManager&) = delete;

	/** @brief 啟動工作執行緒。 */
	void Start(std::uint32_t ioThreadCount = 2, std::uint32_t decodeThreadCount = 2);

	/** @brief 停止所有執行緒。 */
	void Stop();

	/** @brief 提交串流請求。 */
	void SubmitRequest(const NNStreamingRequest& request);

	/** @brief 取消指定 GUID 的請求。 */
	void CancelRequest(NNGuid guid);

	/** @brief 取得待處理請求數量。 */
	std::uint32_t GetPendingRequestCount() const;

	/** @brief 取得完成佇列大小（供 Tick() 檢查）。 */
	std::uint32_t GetCompletedCount() const;

	/** @brief 取出一個完成結果（非阻塞，佇列為空回傳 false）。 */
	bool PopCompleted(NNAsyncIoResult& outResult);

	/** @brief IO 執行緒函式。 */
	void IoThreadFunc();

	/** @brief 解碼執行緒函式。 */
	void DecodeThreadFunc();

private:
	/* 請求佇列（優先級堆） */
	mutable std::mutex requestMutex_;
	std::condition_variable requestCv_;
	std::priority_queue<NNStreamingRequest> requestQueue_;

	/* 解碼佇列（IO 完成後送入） */
	mutable std::mutex decodeMutex_;
	std::condition_variable decodeCv_;
	std::queue<NNAsyncIoResult> decodeQueue_;

	/* 完成佇列（Decode 完成後送入，主線程 Tick() 消費） */
	mutable std::mutex completionMutex_;
	std::queue<NNAsyncIoResult> completionQueue_;

	/* 執行緒池 */
	std::vector<std::thread> ioThreads_;
	std::vector<std::thread> decodeThreads_;
	std::atomic<bool> running_{false};
};

} // namespace NN::Runtime::Asset
