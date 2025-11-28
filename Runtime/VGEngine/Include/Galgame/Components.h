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
#include "../Interface/SceneInterface.h"
#include "../Asset/Accessor/ISceneSerializer.h"

namespace VisionGal::GalGame
{
	struct VG_ENGINE_API GalGameEngineComponent: public IComponent
	{
		GalGameEngineComponent();
		~GalGameEngineComponent() override = default;

		GalGameEngineComponent(const GalGameEngineComponent&) = default;
		GalGameEngineComponent& operator=(const GalGameEngineComponent&) = default;
		GalGameEngineComponent(GalGameEngineComponent&&) noexcept = default;
		GalGameEngineComponent& operator=(GalGameEngineComponent&&) noexcept = default;

		// 通过 IComponent 继承 
		std::string GetComponentType() const override;

		struct DeserializeData
		{
			String m_ScriptPath;
		};

		DeserializeData __DeserializeData;

		template <class Archive>
		void save(Archive& archive) const
		{
			saveIComponent(archive);
			//if (script != nullptr)
			//{
			//	archive(cereal::make_nvp("HasScript", true));
			//	archive(cereal::make_nvp("m_ScriptPath", scriptPath));
			//}
			//else
			//{
			//	archive(cereal::make_nvp("HasScript", false));
			//}

			archive(cereal::make_nvp("m_ScriptPath", scriptPath));
		}

		template<class Archive>
		void load(Archive& archive) {
			loadIComponent(archive);
			archive(__DeserializeData.m_ScriptPath);
		}

		std::string scriptPath;
		//Ref<LuaStoryScript> script;
	};

	struct GalGameEngineComponentSerializer : public IEntityComponentSerializer<GalGameEngineComponent>
	{
		GalGameEngineComponentSerializer() = default;
		GalGameEngineComponentSerializer(const GalGameEngineComponentSerializer&) = default;
		GalGameEngineComponentSerializer& operator=(const GalGameEngineComponentSerializer&) = default;
		GalGameEngineComponentSerializer(GalGameEngineComponentSerializer&&) noexcept = default;
		GalGameEngineComponentSerializer& operator=(GalGameEngineComponentSerializer&&) noexcept = default;
		~GalGameEngineComponentSerializer() override = default;

		void AddActorSerializeComponent(Scene* scene, GameActor* actor, VGActorID id) override;
	};
}