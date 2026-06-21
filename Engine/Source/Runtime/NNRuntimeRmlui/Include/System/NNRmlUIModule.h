#pragma once

/**
 * @file NNRmlUIModule.h
 * @brief RmlUI 模块入口——提供 NNRmlUISystem 创建和管理。
 *
 * NNRmlUISystem 不是 ISceneSystem（用户架构审查反馈：UI 是 Renderer Pipeline Feature）。
 * 由 ViewportRenderRuntimeApi 创建和持有。
 */

#include "../../VGUIConfig.h"

namespace NN::Runtime::RmlUI
{
	class NNRmlUISystem;

	/// @brief 创建 NNRmlUISystem 实例（调用方负责管理生命周期）。
	VG_UI_API NNRmlUISystem* CreateRmlUISystem();

	/// @brief 销毁 NNRmlUISystem 实例。
	VG_UI_API void DestroyRmlUISystem(NNRmlUISystem* system);

} // namespace NN::Runtime::RmlUI
