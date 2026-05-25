#pragma once

/**
 * @file NNJsonSceneSerializer.h
 * @brief Editor 人类可读 JSON 场景序列化器。
 *
 * 与 NNSceneSerializer（二进制 runtime snapshot）并行存在：
 * - Binary serializer → runtime loading / streaming / networking
 * - JSON serializer  → editor scene / git diff / merge / prefab / debugging
 *
 * 完全复用 NNComponentRegistry 字段反射，零硬编码组件类型。
 * 依赖 nlohmann/json（通过 NevernessCore-Core 传递可用）。
 */

#include <string>

#include "../Scene/NNEntity.h"
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	class NNRuntimeScene;

	/**
	 * @brief JSON 场景序列化器（无状态静态 API）。
	 *
	 * 格式版本 1：每个实体输出 id + components 对象，
	 * 组件 key 为注册名（如 "Transform"），字段 key 为字段名（如 "Position"）。
	 */
	class NN_RUNTIME_SCENE_API NNJsonSceneSerializer
	{
	public:
		/** @brief 当前 JSON 格式版本。 */
		static constexpr std::uint32_t kFormatVersion = 1u;

		/** @brief 场景 → 格式化 JSON 字符串（4 空格缩进）。 */
		[[nodiscard]] static std::string Serialize(const NNRuntimeScene& scene);

		/** @brief JSON 字符串 → 场景（追加实体到空或已清空的场景）。 */
		static bool Deserialize(
			NNRuntimeScene& scene,
			const std::string& json);
	};
} // namespace NN::Runtime::Scene
