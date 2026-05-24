#pragma once

/**
 * @file NNPrefab.h
 * @brief Prefab 实例化（Phase 7）。
 *
 * 从 .nnasset 的 EntityHierarchy blob 数据实例化实体子图到 NNRuntimeScene。
 * 二进制格式与 C# PrefabImporter.SerializePrefab 对齐。
 */

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../Scene/NNEntity.h"
#include "../../NNRuntimeScene/NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	class NNRuntimeScene;

	/**
	 * @brief Prefab 实例化器。
	 *
	 * 使用方法：
	 *   NNEntity root = NNEntityInvalid;
	 *   NNPrefab::Instantiate(scene, blobData, blobSize, root);
	 */
	class NN_RUNTIME_SCENE_API NNPrefab
	{
	public:
		/**
		 * @brief 从 EntityHierarchy blob 数据实例化 Prefab 到目标场景。
		 * @param scene         目标运行时场景
		 * @param blobData      EntityHierarchy blob 指针
		 * @param blobSize      blob 大小（字节）
		 * @param outRootEntity [out] 实例化后的根实体句柄
		 * @return 是否成功
		 */
		static bool Instantiate(
			NNRuntimeScene& scene,
			const void* blobData,
			std::size_t blobSize,
			NNEntity& outRootEntity);

		/**
		 * @brief 从 blob 向量实例化。
		 */
		static bool Instantiate(
			NNRuntimeScene& scene,
			const std::vector<std::uint8_t>& blob,
			NNEntity& outRootEntity);
	};
} // namespace NN::Runtime::Scene
