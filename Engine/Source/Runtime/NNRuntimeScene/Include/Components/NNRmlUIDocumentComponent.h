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
 * - RuntimeHandle 为瞬态字段，不序列化（由 UISystem 管理）
 *
 * 与 Legacy RmlUIDocumentComponent 的区别：
 * - 移除 Ref<RmlUIDocument> document（非 POD）
 * - 移除 __DeserializeData（Legacy 序列化专用）
 * - 新增 SortOrder（多文档渲染排序）
 * - 新增 AutoLoad / Visible / Focusable / ReceivesInput 标志位
 */

#include <cstdint>
#include <type_traits>
#include "../../NNRuntimeSceneExport.h"
#include "../../../NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief RmlUI 文档标志位（可组合位掩码）。
	 */
	enum class NNRmlUIDocumentFlags : std::uint32_t
	{
		None          = 0,
		AutoLoad      = 1u << 0,  ///< 场景启动时自动加载文档
		Visible       = 1u << 1,  ///< 初始可见
		Focusable     = 1u << 2,  ///< 可接收焦点
		ReceivesInput = 1u << 3,  ///< 可接收输入事件
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
	 * @brief RmlUI 文档组件：纯数据描述，行为由 UISystem 驱动。
	 *
	 * 内存布局（24 字节）：
	 * ┌──────────────────────────────────────────────────────┐
	 * │ DocumentAsset   NNGuid       16B   文档资产 GUID     │
	 * │ Flags           uint32_t      4B   标志位            │
	 * │ SortOrder       int32_t       4B   渲染排序          │
	 * └──────────────────────────────────────────────────────┘
	 * 对齐：4 字节，共 24 字节（6 × 4B）
	 *
	 * 运行时句柄（RuntimeHandle）由 UISystem 管理，不序列化。
	 */
	struct NN_RUNTIME_SCENE_API NNRmlUIDocumentComponent
	{
		NNGuid                DocumentAsset{};           ///< 文档资产 128-bit GUID（全零=未设置）
		NNRmlUIDocumentFlags  Flags = NNRmlUIDocumentFlags::AutoLoad
		                            | NNRmlUIDocumentFlags::Visible
		                            | NNRmlUIDocumentFlags::ReceivesInput;  ///< 标志位
		std::int32_t          SortOrder = 0;             ///< 渲染排序（小的在前，大的在后）

		NNRmlUIDocumentComponent() = default;
	};

	// ── 编译期验证 ──
	static_assert(std::is_trivially_copyable_v<NNRmlUIDocumentComponent>,
		"NNRmlUIDocumentComponent must be trivially copyable (memcpy-safe)");
	static_assert(std::is_standard_layout_v<NNRmlUIDocumentComponent>,
		"NNRmlUIDocumentComponent must have standard layout");
	static_assert(sizeof(NNRmlUIDocumentComponent) == 24,
		"NNRmlUIDocumentComponent must be exactly 24 bytes");

	/**
	 * @brief RmlUI 文档运行时状态（瞬态，不序列化）。
	 *
	 * 由 UISystem 管理，不参与场景序列化/反序列化。
	 * 场景加载后由 UISystem 根据 AutoLoad 标志创建。
	 */
	struct NNRmlUIDocumentRuntimeState
	{
		std::uint32_t RuntimeHandle = 0u;  ///< RmlUI 文档句柄
		bool          IsLoaded = false;     ///< 是否已加载
		bool          IsVisible = false;    ///< 当前是否可见
	};

} // namespace NN::Runtime::Scene
