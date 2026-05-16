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
#include "../EngineConfig.h"
#include "Manager/ViewportManager.h"
#include "Manager/ShaderManager.h"
#include "NNRuntimeAsset/Interface/AssetManager.h"
#include "Manager/SceneManager.h"

namespace VisionGal
{
    VG_ENGINE_API ViewportManager* GetViewportManager();

    VG_ENGINE_API ShaderManager* GetShaderManager();

    VG_ENGINE_API AssetManager* GetAssetManager();
	 
    VG_ENGINE_API SceneManager* GetSceneManager();
}
