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
#include <NNRuntimeImGui/IncludeImGuiEx.h>
#include <NNPlatformCore/Interface/FileSystem/HFileSystem.h>

namespace NN::Editor {
	class NewDirectoryUITask : public ImGuiEx::ImTaskInterface
	{
	public:
		NewDirectoryUITask(const NN::Core::fsPath& parentpath)
			:m_ParentPath(parentpath)
		{
		};

		void RenderUI(TaskContext& context) override;

	private:
		NN::Core::fsPath m_ParentPath;

		std::string m_FileName;

		bool m_Error = false;
	};

}
