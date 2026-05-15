/*
 * SceneRenderSnapshot — 渲染侧「场景视口数据面」快照（Phase 8G 骨架）
 *
 * 中文：目标为 **RenderPipeline** 仅依赖本结构（相机矩阵、分层精灵排序结果、角色混合中间 RT 句柄等），
 * 与 **GalGameRuntimeState** / 对白业务解耦；当前仅占位字段，避免继续把 **ILayeredSceneManager** 强耦合进着色路径。
 */

#pragma once

#include "../VGGalPresentationConfig.h"
#include <cstdint>

namespace VisionGal::GalGame
{
	struct VG_GALGAME_PRESENTATION_API SceneRenderSnapshot
	{
		/// 中文：单调帧号；由宿主在 **BeforeRender** 前递增，用于调试对比。
		std::uint64_t frameSerial = 0;

		/// 中文：占位 —— 后续填入 **Scene*** 或 **World** 的弱引用 ID，而非裸指针跨 DLL。
		std::uint64_t sceneToken = 0;
	};
}
