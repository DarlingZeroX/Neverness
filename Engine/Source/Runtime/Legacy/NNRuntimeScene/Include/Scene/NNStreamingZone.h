#pragma once

/**
 * @file NNStreamingZone.h
 * @brief 场景区域流式加载框架（Phase 7，API/数据结构层）。
 *
 * 定义流式加载区域（Zone）的概念：
 *   - 每个 Zone 对应一个可加载的场景区域
 *   - 根据相机位置判定加载/卸载
 *   - 委托 NNSceneAssetLoader 执行实际加载
 *
 * 设计：
 *   - 纯 API/数据结构，完整运行时逻辑延后
 *   - 支持球形判定区域
 *   - 支持优先级
 */

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "../../../Engine/EngineTypes.h"
#include "../../NNRuntimeScene/NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	class NNRuntimeScene;

	/** @brief 3D 向量（简化版，用于流式判定）。 */
	struct NNFloat3
	{
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
	};

	/** @brief 流式加载区域状态。 */
	enum class NNZoneState : std::uint8_t
	{
		Unloaded = 0,   ///< 未加载
		Loading,        ///< 正在加载
		Loaded,         ///< 已加载
		Unloading       ///< 正在卸载
	};

	/** @brief 流式加载区域定义。 */
	struct NNStreamingZoneDef
	{
		std::string Name{};             ///< 区域名称
		NNGuid SceneAssetGuid{};        ///< 场景资产 GUID
		NNFloat3 Center{};              ///< 区域中心（世界坐标）
		float Radius = 0.f;             ///< 区域半径
		float LoadDistance = 0.f;       ///< 加载触发距离（相机到中心距离 < 此值时加载）
		float UnloadDistance = 0.f;     ///< 卸载触发距离（相机到中心距离 > 此值时卸载）
		bool IsLoaded = false;          ///< 当前是否已加载
		NNRuntimeScene* LoadedScene = nullptr;  ///< 已加载的场景指针（nullptr = 未加载）
	};

	/**
	 * @brief 场景流式加载管理器。
	 *
	 * 使用方法：
	 *   NNSceneStreamingManager manager;
	 *   manager.AddZone({ "Region_01", guid, {0,0,0}, 100.f, 80.f, 120.f });
	 *   // 每帧调用
	 *   manager.Tick(cameraPosition);
	 */
	class NN_RUNTIME_SCENE_API NNSceneStreamingManager
	{
	public:
		NNSceneStreamingManager() = default;

		/** @brief 添加流式区域。 */
		void AddZone(const NNStreamingZoneDef& zone);

		/** @brief 移除流式区域。 */
		void RemoveZone(const std::string& name);

		/** @brief 每帧调用：根据相机位置判定加载/卸载。 */
		void Tick(const NNFloat3& cameraPosition);

		/** @brief 获取已加载区域数量。 */
		[[nodiscard]] std::size_t GetLoadedZoneCount() const;

		/** @brief 获取总区域数量。 */
		[[nodiscard]] std::size_t GetTotalZoneCount() const;

		/** @brief 获取所有区域定义（只读）。 */
		[[nodiscard]] const std::vector<NNStreamingZoneDef>& GetZones() const { return m_Zones; }

		/** @brief 设置区域加载回调（由外部提供实际加载逻辑）。 */
		void SetLoadCallback(std::function<void(NNStreamingZoneDef&)> callback) { m_OnLoadZone = std::move(callback); }

		/** @brief 设置区域卸载回调。 */
		void SetUnloadCallback(std::function<void(NNStreamingZoneDef&)> callback) { m_OnUnloadZone = std::move(callback); }

		/** @brief 清除所有区域。 */
		void Clear();

	private:
		std::vector<NNStreamingZoneDef> m_Zones{};
		std::function<void(NNStreamingZoneDef&)> m_OnLoadZone{};
		std::function<void(NNStreamingZoneDef&)> m_OnUnloadZone{};
		mutable std::mutex m_Mutex{};
	};
} // namespace NN::Runtime::Scene
