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
//#include "VGEditorComponent/Interface.h"
#include <VGGalgameCore/Include/Components.h>
#include <NNEngineLegacy/Include/Scene/Components.h>
#include <NNEditorFramework/Interface/IComponentDrawer.h>

namespace VisionGal::Editor
{
	struct GalGameEngineComponentDrawer: public IComponentDrawer
	{
		~GalGameEngineComponentDrawer() override = default;

		void OnGUI(IEntity* entity) override;
		const String GetBindType() const override;

		void ScriptBeginDropTarget(GalGame::GalGameEngineComponent* com);

		void UIFileBeginDropTarget(std::string& path);
	};
}
