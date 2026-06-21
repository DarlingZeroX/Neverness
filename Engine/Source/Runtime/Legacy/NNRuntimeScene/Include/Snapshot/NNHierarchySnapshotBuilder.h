#pragma once

/**
 * @file NNHierarchySnapshotBuilder.h
 * @brief 层级快照构建器——从 ECS Registry 批量构建 DFS 有序的连续 buffer。
 *
 * 输出格式：[NNSceneSnapshotHeader][NNSceneNodeSnapshot[nodeCount]][char namePool[namePoolBytes]]
 * 完全 POD，可直接 memcpy 到 C#（通过 NNEditorSceneAPI 函数表）。
 *
 * 构建算法：两遍扫描。
 *   第一遍：统计节点数 + namePool 字节数，收集根节点。
 *   第二遍：DFS 遍历，写入 Nodes[] + NamePool。
 */

#include "../Scene/NNRuntimeScene.h"
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 层级快照构建器（无状态静态工具类）。
	 */
	class NN_RUNTIME_SCENE_API NNHierarchySnapshotBuilder
	{
	public:
		/**
		 * @brief 构建完整快照到 outBuffer。
		 * @param scene     场景引用
		 * @param outBuffer 调用方预分配的缓冲区
		 * @param capacity  缓冲区容量（字节）
		 * @return 实际写入字节数；0 = capacity 不足（此时仍计算所需大小供 GetSnapshotSize 使用）。
		 */
		static std::uint32_t Build(
			const NNRuntimeScene& scene,
			void*                 outBuffer,
			std::uint32_t         capacity);

		/**
		 * @brief 获取构建当前快照所需的总字节数（Header + Nodes + NamePool）。
		 */
		static std::uint32_t EstimateSize(const NNRuntimeScene& scene);
	};
} // namespace NN::Runtime::Scene
