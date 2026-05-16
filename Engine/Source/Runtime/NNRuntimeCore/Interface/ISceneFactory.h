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
#include "SceneInterface.h"

namespace VisionGal
{
	class ISceneFactory
	{
	public:
		virtual ~ISceneFactory() = default;

		virtual Ref<IScene> CreateScene() = 0;
	};

	struct IGameActorFactory
	{
		virtual ~IGameActorFactory() = default;

		virtual IGameActor* CreateActor(IScene* scene, const String& type, IEntity* parent = nullptr) = 0;
	};

	class VG_CORE_API SceneFactoryRegistry
	{
	public:
		static void Register(ISceneFactory* factory);

		static ISceneFactory* GetFactory();

	private:
		static SceneFactoryRegistry& Get();

		ISceneFactory* m_Factory = nullptr;
	};

	class VG_CORE_API GameActorFactoryRegistry
	{
	public:
		static void Register(IGameActorFactory* factory);

		static IGameActorFactory* GetFactory();

	private:
		static GameActorFactoryRegistry& Get();

		IGameActorFactory* m_Factory = nullptr;
	};
}
