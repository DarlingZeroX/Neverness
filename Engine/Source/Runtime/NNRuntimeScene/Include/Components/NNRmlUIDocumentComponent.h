#pragma once

/**
 * @file NNRmlUIDocumentComponent.h
 * @brief RmlUI 文档组件（POD，无虚表）：UI Document Data Component。
 *
 * 设计参考：Unity Canvas + UIDocument、Godot Control。
 *
 * 约束：
 * - trivially_copyable + standard_layout（可直接 memcpy）
 * - 无 std::string / shared_ptr / 虚函数
 * - 资源引用使用 128-bit NNGuid（永久稳定资产标识）
 * - 文档实例由 RmlUIRenderer 管理，组件只存数据
 *
 * 与 Legacy RmlUIDocumentComponent 的区别：
 * - 移除 Ref<RmlUIDocument> document（非 POD）
 * - 移除 __DeserializeData（Legacy 序列化专用）
 * - 移除 ResolvedPath（路径由 IAssetResolver 动态解析）
 * - 新增 Visibility（SceneView / GameView / Both）
 * - 新增 SortOrder（多文档渲染排序）
 */

#include <cstdint>
#include <type_traits>
#include "../../NNRuntimeSceneExport.h"
#include "../../../NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief RmlUI 文档视图目标——控制在哪个视图中显示。
	 */
	enum class NNRmlUIViewTarget : std::uint32_t
	{
		Scene = 0,  ///< 仅在 Scene View 中显示（编辑模式预览）
		Game  = 1,  ///< 仅在 Game View 中显示（播放模式）
		Both  = 2,  ///< 两个视图都显示
	};

	/**
	 * @brief RmlUI 文档标志位（可组合位掩码）。
	 */
	enum class NNRmlUIDocumentFlags : std::uint32_t
	{
		None          = 0,
		AutoLoad      = 1u << 0,  ///< 场景启动时自动加载文档
		Focusable     = 1u << 1,  ///< 可接收焦点
		ReceivesInput = 1u << 2,  ///< 可接收输入事件
	};

	inline NNRmlUIDocumentFlags operator|(NNRmlUIDocumentFlags a, NNRmlUIDocumentFlags b) noexcept
	{
		return static_cast<NNRmlUIDocumentFlags>(
			static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	}
	inline NNRmlUIDocumentFlags operator&(NNRmlUIDocumentFlags a, NNRmlUIDocumentFlags b) noexcept
	{
		return static_cast<NNRmlUIDocumentFlags>(
			static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
	}
	inline bool HasFlag(NNRmlUIDocumentFlags flags, NNRmlUIDocumentFlags test) noexcept
	{
		return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(test)) != 0;
	}

	/**
	 * @brief RmlUI 文档组件：纯数据描述，行为由 NNRmlUISystem + RmlUIRenderer 驱动。
	 *
	 * 内存布局（32 字节，含 4B 尾部对齐填充）：
	 * ┌──────────────────────────────────────────────────────┐
	 * │ DocumentAsset   NNGuid              16B   文档资产   │
	 * │ Flags           NNRmlUIDocumentFlags 4B   标志位     │
	 * │ SortOrder       int32_t              4B   渲染排序   │
	 * │ ViewTarget      NNRmlUIViewTarget    4B   视图目标   │
	 * │ _padding        (implicit)           4B   对齐填充   │
	 * └──────────────────────────────────────────────────────┘
	 * 对齐：4 字节，共 32 字节（8 × 4B）
	 *
	 * 序列化：DocumentAsset + Flags + SortOrder + ViewTarget
	 * 运行时：文档实例由 RmlUIRenderer 内部管理，不存于组件。
	 */
	struct NN_RUNTIME_SCENE_API NNRmlUIDocumentComponent
	{
		NNGuid                DocumentAsset{};           ///< 文档资产 128-bit GUID（全零=未设置）
		NNRmlUIDocumentFlags  Flags = NNRmlUIDocumentFlags::AutoLoad
		                            | NNRmlUIDocumentFlags::ReceivesInput;  ///< 标志位
		std::int32_t          SortOrder = 0;             ///< 渲染排序（小的在前，大的在后）
		NNRmlUIViewTarget     ViewTarget = NNRmlUIViewTarget::Both;  ///< 视图目标

		NNRmlUIDocumentComponent() = default;
	};

	// ── 编译期验证 ──
	static_assert(std::is_trivially_copyable_v<NNRmlUIDocumentComponent>,
		"NNRmlUIDocumentComponent must be trivially copyable (memcpy-safe)");
	static_assert(std::is_standard_layout_v<NNRmlUIDocumentComponent>,
		"NNRmlUIDocumentComponent must have standard layout");
	static_assert(sizeof(NNRmlUIDocumentComponent) == 32,
		"NNRmlUIDocumentComponent must be exactly 32 bytes");

} // namespace NN::Runtime::Scene
