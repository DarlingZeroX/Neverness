#pragma once

/**
 * @file NNSceneAssetLoader.h
 * @brief 场景资产异步预加载框架（Phase 7）。
 *
 * 扫描场景组件中的 Guid 字段，收集引用的资产 GUID，
 * 通过 NNAssetManager::LoadAssetAsync() 发起异步加载，
 * 追踪加载进度。
 *
 * 设计：
 *   - 纯 API/框架实现
 *   - 不依赖具体的渲染/音频系统
 *   - 支持优先级调度
 */

#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "../../../Engine/EngineTypes.h"
#include "../../NNRuntimeScene/NNRuntimeSceneExport.h"
#include "../Scene/NNEntity.h"

namespace NN::Runtime::Scene
{
	class NNRuntimeScene;

	/** @brief 资产加载优先级。 */
	enum class NNAssetLoadPriority : std::uint8_t
	{
		Critical = 0,   ///< 立即需要（可见物体的纹理等）
		High = 1,       ///< 相机附近
		Normal = 2,     ///< 默认
		Low = 3,        ///< 后台预加载
		Background = 4  ///< 最低优先级
	};

	/** @brief 单个资产的加载状态。 */
	enum class NNAssetLoadState : std::uint8_t
	{
		Pending = 0,
		Loading,
		Loaded,
		Failed
	};

	/** @brief 预加载请求。 */
	struct NNAssetPreloadRequest
	{
		NNGuid AssetGuid{};
		NNAssetLoadPriority Priority = NNAssetLoadPriority::Normal;
		NNAssetLoadState State = NNAssetLoadState::Pending;
	};

	/**
	 * @brief 场景资产预加载器。
	 *
	 * 使用方法：
	 *   NNSceneAssetLoader loader;
	 *   loader.ScanScene(scene);
	 *   loader.StartLoading();
	 *   // 每帧调用
	 *   loader.Tick();
	 *   if (loader.IsAllLoaded()) { ... }
	 */
	class NN_RUNTIME_SCENE_API NNSceneAssetLoader
	{
	public:
		NNSceneAssetLoader() = default;

		/** @brief 扫描场景中所有组件的 Guid 字段，收集资产引用。 */
		void ScanScene(const NNRuntimeScene& scene);

		/** @brief 开始异步加载所有收集到的资产。 */
		void StartLoading();

		/** @brief 每帧调用：检查加载状态，更新进度。 */
		void Tick();

		/** @brief 所有资产是否已加载完成。 */
		[[nodiscard]] bool IsAllLoaded() const;

		/** @brief 加载进度（0.0 ~ 1.0）。 */
		[[nodiscard]] float GetProgress() const;

		/** @brief 待加载资产总数。 */
		[[nodiscard]] std::size_t GetTotalCount() const;

		/** @brief 已完成加载的资产数。 */
		[[nodiscard]] std::size_t GetLoadedCount() const;

		/** @brief 获取所有请求列表（只读）。 */
		[[nodiscard]] const std::vector<NNAssetPreloadRequest>& GetRequests() const { return m_Requests; }

		/** @brief 设置加载完成回调。 */
		void SetOnAllLoadedCallback(std::function<void()> callback) { m_OnAllLoaded = std::move(callback); }

		/** @brief 清除所有状态。 */
		void Reset();

	private:
		std::vector<NNAssetPreloadRequest> m_Requests{};
		std::unordered_map<std::uint64_t, std::size_t> m_GuidToIndex{};  // guid.low -> m_Requests 下标
		std::function<void()> m_OnAllLoaded{};
		mutable std::mutex m_Mutex{};
		bool m_Loading = false;
	};
} // namespace NN::Runtime::Scene
