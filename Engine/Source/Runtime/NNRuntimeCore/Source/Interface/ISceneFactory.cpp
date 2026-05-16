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

#include "ISceneFactory.h"

namespace NN::Runtime
{
	void SceneFactoryRegistry::Register(ISceneFactory* factory)
	{
		Get().m_Factory = factory;
	}

	ISceneFactory* SceneFactoryRegistry::GetFactory()
	{
		return Get().m_Factory;
	}

	SceneFactoryRegistry& SceneFactoryRegistry::Get()
	{
		static SceneFactoryRegistry instance;
		return instance;
	}

	void GameActorFactoryRegistry::Register(IGameActorFactory* factory)
	{
		Get().m_Factory = factory;
	}

	IGameActorFactory* GameActorFactoryRegistry::GetFactory()
	{
		return Get().m_Factory;
	}

	GameActorFactoryRegistry& GameActorFactoryRegistry::Get()
	{
		static GameActorFactoryRegistry instance;
		return instance;
	}
}
