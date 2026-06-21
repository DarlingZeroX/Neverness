#pragma once

/**
 * @file NNScriptComponent.h
 * @brief 脚本组件（POD，无虚表）：关联 Entity 与 C# 脚本类型。
 *
 * 设计原则：
 * - 只保存可序列化状态（ScriptTypeId + Enabled）
 * - 运行时实例映射（Behaviour 指针、GCHandle）由 C# ScriptBehaviourScheduler 管理
 * - 当前限制：一个 Entity 只挂一个 NNScriptComponent
 *
 * ⚠️ ComponentTypeId 与 ScriptTypeId 是完全不同的概念：
 * - ComponentTypeId = FNV1a64("Script")（组件类型标识，用于 ECS 注册/ABI 调用）
 * - ScriptTypeId = FNV1a64(Type.FullName)（脚本类标识，用于实例化/序列化持久化）
 *
 * 约束：
 * - trivially_copyable + standard_layout（可直接 memcpy）
 * - 无 std::string / shared_ptr / 虚函数
 */

#include <cstdint>
#include <type_traits>

#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 脚本组件：关联 Entity 与 C# 脚本类型，行为由 ScriptBehaviourScheduler 驱动。
	 *
	 * 内存布局（16 字节）：
	 * ┌──────────────────────────────────────────────────────┐
	 * │ ScriptTypeId   uint64_t   8B   脚本类型 ID           │
	 * │                          （FNV1a64(Type.FullName)）   │
	 * │ Enabled        uint8_t    1B   启用状态               │
	 * │ _reserved      uint8_t[7] 7B   对齐填充               │
	 * └──────────────────────────────────────────────────────┘
	 * 对齐：8 字节，共 16 字节（2 × 8B）
	 *
	 * 序列化字段：
	 * - ScriptTypeId: 脚本类型 FNV1a64 hash（可序列化、可持久化）
	 * - Enabled: 启用状态（可序列化）
	 *
	 * C# ABI 镜像：NNScriptComponentData（Neverness.Runtime.Engine）
	 * C# 高层 API：ScriptComponent（Neverness.Gameplay）
	 */
	struct NN_RUNTIME_SCENE_API NNScriptComponent
	{
		std::uint64_t ScriptTypeId = 0u;   ///< 脚本类型 ID（FNV1a64(Type.FullName)，0 = 未设置）
		std::uint8_t  Enabled = 1u;        ///< 启用状态（1 = 启用，0 = 禁用）
		std::uint8_t  _reserved[7] = {};   ///< 对齐填充

		NNScriptComponent() = default;
	};

	// ── 编译期验证 ──
	static_assert(std::is_trivially_copyable_v<NNScriptComponent>,
		"NNScriptComponent must be trivially copyable (memcpy-safe)");
	static_assert(std::is_standard_layout_v<NNScriptComponent>,
		"NNScriptComponent must have standard layout");
	static_assert(sizeof(NNScriptComponent) == 16,
		"NNScriptComponent must be exactly 16 bytes");

} // namespace NN::Runtime::Scene
