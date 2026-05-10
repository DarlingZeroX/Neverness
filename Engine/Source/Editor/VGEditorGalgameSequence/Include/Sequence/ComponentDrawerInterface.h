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
#include <VGEditorFramework/Interface/UIInterface.h>
#include <VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h>

namespace VisionGal::Editor
{
	struct IGalSeqComDrawer
	{
		virtual ~IGalSeqComDrawer() = default;

		virtual void OnGUI(unsigned int index, IVGSSequenceComponent* obj) = 0;
		virtual const std::string GetBindType() const = 0;
	};
}
