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

#include <memory>
#include <string>

#include "../VGGalEditorConfig.h"
#include "../Include/Sequence/SequenceComponentTableUI.h"
#include "VGEditorFramework/Interface/UITaskInterface.h"
#include "VGGalgameScriptSequence/Include/Sequence/DataContainer.h"

namespace VisionGal::Editor
{
	class VG_GALGAME_EDITOR_API VGScriptSequenceEditor : public IEditorTaskPanel
	{
	public:
		VGScriptSequenceEditor();
		VGScriptSequenceEditor(const std::string& path);
		~VGScriptSequenceEditor() override;

		bool SaveAsset();
		bool OpenAsset(const std::string& path);

		void RenderSequenceUI();
		void RenderUI(TaskContext& context) override;

		void SaveTest();
	private:
		std::string m_AssetPath;
		Ref<VGSSequenceDataContainer> m_SequenceData;

		SequenceComponentUI m_EntryUI;
	};

}
