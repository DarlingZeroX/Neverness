/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once
#include "../VGGRCExport.h"
#include "VGGalgameContract/VGGalCoreConfig.h"
#include "VGCore/Include/Data/DataContainer.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief 存档用变量容器；带稳定 schema 元数据（Phase 7 存档 ABI）。
	 *
	 * **schemaVersion**：与 SaveArchive / ValidateArchiveSchema 对齐的容器格式版本。
	 * **schemaHash**：预留，用于校验 Choices/Inputs 命名空间结构（当前可为 0）。
	 */
	class VG_RUNTIME_GALCORE_API ArchiveDataContainer: public VGDataContainer
	{
	public:
		ArchiveDataContainer() = default;
		~ArchiveDataContainer() = default;

		int schemaVersion = 1;
		std::uint64_t schemaHash = 0;

		VGDataNamespace* GetChoicesNamespace();
		VGDataNamespace* GetInputNamespace();

		static void InitializeLuaBinding(sol::table& L);
	};
}