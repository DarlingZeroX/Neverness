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
#include "VGCore/Interface/GameInterface.h"
#include "VGCore/Interface/SceneInterface.h"

namespace VisionGal
{
    struct IGameActorFactory
    {
        virtual ~IGameActorFactory() = default;

        virtual IGameActor* CreateActor(IScene* scene, const String& type, IEntity* parent = nullptr) = 0;
    };

    class GameActorFactory: public IGameActorFactory
    {
    public:
        GameActorFactory();
        ~GameActorFactory() override = default;
        GameActorFactory(const GameActorFactory&) = delete;
        GameActorFactory& operator=(const GameActorFactory&) = delete;
        GameActorFactory(GameActorFactory&&) noexcept = default;
        GameActorFactory& operator=(GameActorFactory&&) noexcept = default;

        IGameActor* CreateActor(IScene* scene, const String& type, IEntity* parent = nullptr) override;

        std::vector<String>& GetActorTypeList();
    private:
        std::vector<String> m_ActorTypeList;
    };

	VG_ENGINE_API GameActorFactory* GetGameActorFactory();
}
