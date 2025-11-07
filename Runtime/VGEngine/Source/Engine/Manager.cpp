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

#include "Engine/Manager.h"

namespace VisionGal
{
	ViewportManager* GetViewportManager()
	{
		return ViewportManager::Get();
	}

	ShaderManager* GetShaderManager()
	{
		return ShaderManager::Get();
	}

	AssetManager* GetAssetManager()
	{
		return AssetManager::GetInstance();
	} 

	SceneManager* GetSceneManager()
	{
		return SceneManager::Get();
	}
}
