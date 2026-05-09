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
#include "../Config.h"
#include <VGCore/Include/Core/Window.h>
#include <VGEngine/Include/Game/GameEngine.h>

namespace VisionGal::Editor
{
	class VG_EDITOR_FRAMEWORK_API ModuleEditorFramework
	{
	public:
		static void MountToEditor(Ref<VGWindow>& editorWindow, Ref<CoreGameEngine>& gameEngine);

	private:

	};


}
