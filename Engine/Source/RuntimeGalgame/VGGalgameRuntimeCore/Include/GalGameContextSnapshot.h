/*
 * GalGameContextSnapshot — 冻结帧 / 调试回放用数据切片（Phase 7.8）
 *
 * 中文：默认浅拷贝 archiveData 引用；不拷贝 Engine 指针（由调用方在 Apply 后自行恢复宿主引用策略）。
 */

#pragma once

#include "GalGameRuntimeState.h"
#include "ArchiveDataContainer.h"
#include "VGCore/Include/Core/Core.h"

namespace VisionGal::GalGame
{
	struct GalGameContext;

	struct GalGameContextSnapshot
	{
		GalGameRuntimeState runtimeState{};
		Ref<ArchiveDataContainer> archiveData;

		static GalGameContextSnapshot Capture(const GalGameContext& src);

		void Apply(GalGameContext& dst) const;
	};
}
