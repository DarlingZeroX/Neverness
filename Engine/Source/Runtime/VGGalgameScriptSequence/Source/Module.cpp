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

#include "Asset/Asset.h"
#include "Asset/AssetFactory.h"
#include "VGAsset/Interface/AssetFactory.h"
#include "VGGalgameRuntime/Interface/IStoryScript.h"
#include "ExecutorCreator.h"

namespace VisionGal::GalGame
{
	void GalGameSequenceScriptModule::MountEngineRuntime()
	{
		// 注册Visual Script的Asset Factory
		EngineAssetFactory::Get().RegisterFactory(MakeScope<GalGameSequenceScriptAssetFactory>());

		GalGameScriptExecutorFactory::Get().RegisterAssetExecutor(
			SequenceScriptAssetType{}.GetNameID(),
			MakeRef<SSExecutorCreatorSequence>() 
		);
	}

}
