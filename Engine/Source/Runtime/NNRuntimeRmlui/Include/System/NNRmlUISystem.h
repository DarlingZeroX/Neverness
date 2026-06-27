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

} // namespace NN::Runtime::RmlUI
