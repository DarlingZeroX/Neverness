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

#include "Galgame/Components.h"

namespace VisionGal::GalGame
{
	GalGameEngineComponent::GalGameEngineComponent()
	{
	}

	std::string GalGameEngineComponent::GetComponentType() const
	{
		return "GalGameEngine";
	}

	void GalGameEngineComponentSerializer::AddActorSerializeComponent(Scene* scene, IGameActor* actor, VGActorID id)
	{
		auto* world = scene->GetWorld();
		GalGameEngineComponent& com = world->emplace<GalGameEngineComponent>(actor->GetEntity());
		GalGameEngineComponent& deserializeComponent = m_ComponentMap[id];

		com = deserializeComponent;
		//if (deserializeComponent.__DeserializeData.HasScript)
		//{
		//	com.script = GalGame::LuaStoryScript::LoadFromFile(deserializeComponent.__DeserializeData.m_ScriptPath);
		//}
		//com.scriptPath = deserializeComponent.__DeserializeData.m_ScriptPath;

		scene->UpdateDeserializeComponent(actor, &com);
	}
}
