#pragma once

/**
 * @file NNRmlUISystem.h
 * @brief RmlUI 系统——负责文档生命周期管理，输出纯数据 DrawList。
 *
 * 职责：
 * - 维护 Entity → 组件快照映射（用于 diff）
 * - 输出 RmlDrawList（按 SortOrder 排序的纯数据列表）
 *
 * 不负责：
 * - Rml::Context / ElementDocument（由 RmlUIRenderer 持有）
 * - 渲染（由 RmlUIRenderer 负责）
 * - 输入处理（由 Editor 侧负责）
 *
 * 已移除：
 * - ECS Query（NNRuntimeScene 移至 Legacy）
 * - NNRuntimeScene& 参数
 */

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include "../../VGUIConfig.h"
#include "RmlUITypes.h"

namespace NN::Runtime::RmlUI
{
	/**
	 * @brief RmlUI 绘制项——纯数据，无 Rml SDK 依赖。
	 *
	 * 由 NNRmlUISystem 或 C# RenderCommands 构建，供 RmlUIRenderer 消费。
	 * 使用 NNEntity（uint64_t）防止 Entity 复用导致旧 Document 绑定新 Entity。
	 *
	 * RenderCommands 路径：C# 直接传 assetPath，不经过 IAssetResolver。
	 */
	struct RmlDrawItem
	{
		NNEntity              entity = 0;             ///< 实体句柄
		NNGuid                assetGuid{};            ///< 文档资产 GUID（旧路径使用）
		std::string           assetPath;              ///< VFS 路径（RenderCommands 路径直接传入）
		std::int32_t          sortOrder = 0;          ///< 渲染排序
		NNRmlUIViewTarget     viewTarget = NNRmlUIViewTarget::Both;  ///< 视图目标
		std::uint32_t         viewportId = 0;         ///< 预留：视口 ID
	};

	/**
	 * @brief RmlUI 系统——只负责构建 DrawList。
	 *
	 * 每帧 Tick：
	 * 1. 同步外部传入的 DrawItem 列表（由 C# RenderCommands 或外部调用方提供）
	 * 2. 与上一帧快照 diff
	 * 3. 重建 DrawList（仅在脏时）
	 * 4. 按 SortOrder 排序
	 *
	 * Renderer 通过 GetDrawList() 读取本帧数据。
	 *
	 * 已移除：ECS Query（NNRuntimeScene 移至 Legacy）。
	 * 现在由外部调用方（C# RenderCommands）直接传入 DrawItem 列表。
	 */
	class VG_UI_API NNRmlUISystem
	{
	public:
		NNRmlUISystem() = default;
		~NNRmlUISystem() = default;

		NNRmlUISystem(const NNRmlUISystem&) = delete;
		NNRmlUISystem& operator=(const NNRmlUISystem&) = delete;

		/// @brief 每帧调用，同步状态并构建 DrawList。
		void Tick(float deltaTimeSeconds);

		/// @brief 设置 DrawItem 列表（由外部调用方提供，替代原 ECS Query）。
		void SetDrawItems(const std::vector<RmlDrawItem>& items);

		/// @brief 获取本帧的绘制列表（按 SortOrder 排序）。
		[[nodiscard]] const std::vector<RmlDrawItem>& GetDrawList() const { return m_DrawList; }

	private:
		/// 输出给 Renderer 的绘制列表
		std::vector<RmlDrawItem> m_DrawList;

		/// DrawList 是否需要重建
		bool m_DrawListDirty = true;
	};

} // namespace NN::Runtime::RmlUI
