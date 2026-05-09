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
#include "ComponentDrawerInterface.h"

namespace VisionGal::Editor
{
	struct GSCD_CommonDialogue : public IGalSeqComDrawer
	{
		~GSCD_CommonDialogue() override = default;

		void OnGUI(unsigned int index,IVGSSequenceComponent* entry) override;
		const std::string GetBindType() const override;
	};

	struct GSCD_ChangeFigure : public IGalSeqComDrawer
	{
		~GSCD_ChangeFigure() override = default;

		void OnGUI(unsigned int index,IVGSSequenceComponent* entry) override;
		const std::string GetBindType() const override;
	};

	struct GSCD_ChangeBackground : public IGalSeqComDrawer
	{
		~GSCD_ChangeBackground() override = default;

		void OnGUI(unsigned int index,IVGSSequenceComponent* entry) override;
		const std::string GetBindType() const override;

		void TextureBeginDropTarget(IVGSSequenceComponent* entry);
	};
}