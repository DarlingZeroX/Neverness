#pragma once
#include "../Interface/SceneInterface.h"
#include "StoryScript.h"
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
			bool HasScript = false;
			String m_ScriptPath;
		};

		DeserializeData __DeserializeData;

		template <class Archive>
		void save(Archive& archive) const
		{
			saveIComponent(archive);
			if (script != nullptr)
			{
				archive(cereal::make_nvp("HasScript", true));
				archive(cereal::make_nvp("m_ScriptPath", script->GetResourcePath()));
			}
			else
			{
				archive(cereal::make_nvp("HasScript", false));
			}
		}

		template<class Archive>
		void load(Archive& archive) {
			loadIComponent(archive);
			archive(__DeserializeData.HasScript);

			if (__DeserializeData.HasScript) {
				archive(__DeserializeData.m_ScriptPath);
			}
			else {
				script = nullptr;
			}
		}

		Ref<LuaStoryScript> script;
	};

	struct GalGameEngineComponentSerializer : public IEntityComponentSerializer<GalGameEngineComponent>
	{
		GalGameEngineComponentSerializer() = default;
		GalGameEngineComponentSerializer(const GalGameEngineComponentSerializer&) = default;
		GalGameEngineComponentSerializer& operator=(const GalGameEngineComponentSerializer&) = default;
		GalGameEngineComponentSerializer(GalGameEngineComponentSerializer&&) noexcept = default;
		GalGameEngineComponentSerializer& operator=(GalGameEngineComponentSerializer&&) noexcept = default;
		~GalGameEngineComponentSerializer() override = default;

		void AddActorSerializeComponent(Scene* scene, GameActor* actor, EntityID id) override;
	};
}