#pragma once

/**
 * @file NNSceneSerializer.h
 * @brief 基于组件注册表字段反射的场景二进制快照（Phase 4-B：FNV-1a 稳定 TypeId）。
 *
 * 格式：魔数 `VGSC` + version + 实体列表（组件 blob 按字段描述拼接，TypeId = FNV-1a name hash）。
 * 反序列化分配新 **NNEntity**（Index/Generation 由运行时重新分配）。
 */

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../Scene/NNEntity.h"
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	class NNRuntimeScene;

	/** @brief 反序列化结果：存档实体下标 → 运行时 **NNEntity**。 */
	using NNSceneEntityArchiveMap = std::unordered_map<std::uint32_t, NNEntity>;

	/**
	 * @brief 场景序列化器（无状态静态 API）。
	 */
	class NN_RUNTIME_SCENE_API NNSceneSerializer
	{
	public:
		/** @brief 格式版本 2 = FNV-1a name hash 作为 TypeId（8 字节）。 */
		static constexpr std::uint32_t kFormatVersion = 2u;

		[[nodiscard]] static std::vector<std::uint8_t> Serialize(const NNRuntimeScene& scene);

		/**
		 * @brief 将快照写入空或已清空的场景（追加实体，不销毁既有实体）。
		 * @param outArchiveMap 可选；返回存档下标到新 Handle 的映射。
		 */
		static bool Deserialize(
			NNRuntimeScene& scene,
			const std::vector<std::uint8_t>& buffer,
			NNSceneEntityArchiveMap* outArchiveMap = nullptr);
	};
} // namespace NN::Runtime::Scene
