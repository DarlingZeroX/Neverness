#pragma once

/**
 * @file NNRmlUISystem.h
 * @brief RmlUI ECS 系统——负责文档生命周期管理，输出纯数据 DrawList。
 *
 * 职责：
 * - 监听 NNRmlUIDocumentComponent 变更
 * - 维护 NNEntity → 组件快照映射（用于 diff）
 * - 输出 RmlDrawList（按 SortOrder 排序的纯数据列表）
 *
 * 不负责：
 * - Rml::Context / ElementDocument（由 RmlUIRenderer 持有）
 * - 渲染（由 RmlUIRenderer 负责）
 * - 输入处理（由 Editor 侧负责）
 * - PlayMode 判断（由 Editor 侧负责）
 * - 资产路径解析（通过 IAssetResolver 接口）
 */

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "../../VGUIConfig.h"

// 必须包含完整定义（NNEntity 是 using 别名，不能前向声明）
#include "NNNativeEngineAPI/Include/EngineTypes.h"
#include "NNRuntimeScene/Include/Scene/NNEntity.h"
#include "NNRuntimeScene/Include/Scene/NNRuntimeScene.h"
#include "NNRuntimeScene/Include/Components/NNRmlUIDocumentComponent.h"

namespace NN::Runtime::RmlUI
{
	using namespace NN::Runtime::Scene;

	/**
	 * @brief NNEntity 哈希（用于 unordered_map）。
	 */
	struct NNEntityHash
	{
		std::size_t operator()(NNEntity e) const noexcept
		{
			return std::hash<std::uint64_t>{}(e);
		}
	};

	/**
	 * @brief RmlUI 绘制项——纯数据，无 Rml SDK 依赖。
	 *
	 * 由 NNRmlUISystem 构建，供 RmlUIRenderer 消费。
	 * 使用 NNEntity（带 generation）防止 Entity 复用导致旧 Document 绑定新 Entity。
	 */
	struct RmlDrawItem
	{
		NNEntity              entity = 0;             ///< 实体句柄（带 generation）
		NNGuid                assetGuid{};            ///< 文档资产 GUID
		std::int32_t          sortOrder = 0;          ///< 渲染排序
		NNRmlUIViewTarget     viewTarget = NNRmlUIViewTarget::Both;  ///< 视图目标
	};

	/**
	 * @brief RmlUI ECS 系统——只负责构建 DrawList。
	 *
	 * 每帧 Tick：
	 * 1. 扫描 ECS 中所有 NNRmlUIDocumentComponent
	 * 2. 与上一帧快照 diff
	 * 3. 重建 DrawList（仅在脏时）
	 * 4. 按 SortOrder 排序
	 *
	 * Renderer 通过 GetDrawList() 读取本帧数据。
	 */
	class VG_UI_API NNRmlUISystem
	{
	public:
		NNRmlUISystem() = default;
		~NNRmlUISystem() = default;

		NNRmlUISystem(const NNRmlUISystem&) = delete;
		NNRmlUISystem& operator=(const NNRmlUISystem&) = delete;

		/// @brief 每帧调用，同步 ECS 组件状态并构建 DrawList。
		void Tick(NNRuntimeScene& scene, float deltaTimeSeconds);

		/// @brief 获取本帧的绘制列表（按 SortOrder 排序）。
		[[nodiscard]] const std::vector<RmlDrawItem>& GetDrawList() const { return m_DrawList; }

	private:
		/// @brief 组件快照（用于 diff 检测）。
		struct EntrySnapshot
		{
			NNGuid assetGuid{};
			std::int32_t sortOrder = 0;
			NNRmlUIViewTarget viewTarget = NNRmlUIViewTarget::Both;
			NNRmlUIDocumentFlags flags{};
			bool alive = false;
		};

		/// @brief 同步 ECS 组件状态。
		void SyncDocuments(NNRuntimeScene& scene);

		/// NNEntity → 组件快照
		std::unordered_map<NNEntity, EntrySnapshot, NNEntityHash> m_Snapshots;

		/// 输出给 Renderer 的绘制列表
		std::vector<RmlDrawItem> m_DrawList;

		/// DrawList 是否需要重建
		bool m_DrawListDirty = true;
	};

} // namespace NN::Runtime::RmlUI
