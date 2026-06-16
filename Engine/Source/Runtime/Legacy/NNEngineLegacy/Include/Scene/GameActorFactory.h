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
#include "NNRuntimeCore/Interface/GameInterface.h"
#include "NNRuntimeCore/Interface/ISceneFactory.h"
#include "NNRuntimeCore/Interface/SceneInterface.h"

namespace NN::Runtime
{
	struct IGameActorBuilder
	{
		virtual ~IGameActorBuilder() = default;

		virtual std::string GetType() const = 0;
		virtual IGameActor* BuildActor(IGameActor* emptyActor) = 0;
	};

    class VG_ENGINE_API GameActorFactory: public IGameActorFactory
    {
    public:
        GameActorFactory();
        ~GameActorFactory() override = default;
        GameActorFactory(const GameActorFactory&) = delete;
        GameActorFactory& operator=(const GameActorFactory&) = delete;
        GameActorFactory(GameActorFactory&&) noexcept = default;
        GameActorFactory& operator=(GameActorFactory&&) noexcept = default;

        IGameActor* CreateActor(IScene* scene, const String& type, IEntity* parent = nullptr) override;

		void AddGameActorCreator(const Ref<IGameActorBuilder>& creator);

        std::vector<String>& GetActorTypeList();
    private:
        std::vector<String> m_ActorTypeList;
		std::vector<Ref<IGameActorBuilder>> m_ActorCreators;
    };

	VG_ENGINE_API GameActorFactory* GetGameActorFactory();
}
