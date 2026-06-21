#pragma once

/**
 * @file NNReflectionSnapshotBuilder.h
 * @brief 反射类型信息快照构建器——从 NNComponentRegistry 批量构建连续 POD buffer。
 *
 * 输出格式：[NNSceneSnapshotHeader][NNEditorComponentInfo[]][NNEditorFieldInfo[]][char namePool[]]
 * 完全 POD，可直接 memcpy 到 C#（通过 NNEditorSceneAPI 函数表）。
 *
 * 与 NNHierarchySnapshotBuilder 并行存在：
 * - Hierarchy snapshot → Scene Hierarchy Tree UI
 * - Reflection snapshot → Inspector / Property Grid / Component Viewer
 *
 * 构建算法：两遍扫描。
 *   第一遍：统计类型数 + 字段总数 + namePool 字节数。
 *   第二遍：顺序写入 ComponentInfo[] + FieldInfo[] + NamePool。
 *
 * 所有数据来自 NNComponentRegistry，零硬编码组件类型。
 */

#include "../Scene/NNRuntimeScene.h"
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 反射类型信息快照构建器（无状态静态工具类）。
	 */
	class NN_RUNTIME_SCENE_API NNReflectionSnapshotBuilder
	{
	public:
		/**
		 * @brief 构建完整类型信息快照到 outBuffer。
		 * @param scene     场景引用
		 * @param outBuffer 调用方预分配的缓冲区
		 * @param capacity  缓冲区容量（字节）
		 * @return 实际写入字节数；capacity 不足时返回所需大小（供 getTypeInfoSnapshotSize 使用）。
		 */
		static std::uint32_t Build(
			const NNRuntimeScene& scene,
			void*                 outBuffer,
			std::uint32_t         capacity);

		/**
		 * @brief 获取构建类型信息快照所需的总字节数。
		 * @return sizeof(Header) + typeCount*24 + totalFieldCount*24 + namePoolBytes。
		 */
		static std::uint32_t EstimateSize(const NNRuntimeScene& scene);
	};
} // namespace NN::Runtime::Scene
