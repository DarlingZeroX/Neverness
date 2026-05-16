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

#include "../Module.h"
#include "VGGalgameCore/Interface/IStoryScript.h"
#include "SSExecutorCreator.h"
#include "NNRuntimeAsset/Include/GalGameAsset.h"

namespace VisionGal::GalGame
{
	void GalGameLuaScriptModule::MountEngineRuntime()
	{
		GalGameScriptExecutorFactory::Get().RegisterAssetExecutor(
			GLuaScriptAssetType{}.GetNameID(),
			MakeRef<LuaStoryScriptExecutorCreator>() 
		);
	}
}
