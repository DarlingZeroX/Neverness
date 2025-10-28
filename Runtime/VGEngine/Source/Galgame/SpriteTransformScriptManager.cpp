#include "Galgame/SpriteTransformScriptManager.h"

#include "Galgame/GameEngineCore.h"
#include "Galgame/SpriteTransformScript.h"
#include "HCore/Include/Core/HLocalization.h"
#include "HCore/Include/Core/HStringTools.h"
#include "Scene/Components.h"

namespace VisionGal::GalGame {

	SpriteTransformScriptManager* SpriteTransformScriptManager::GetInstance()
	{
		static SpriteTransformScriptManager s_Manager;
		return &s_Manager;
	}

	Ref<IAnimationScript> SpriteTransformScriptManager::CreateSpriteTransformWithCommand(GameActor* actor, const String& cmd)
	{
		if (actor == nullptr)
			return nullptr;

		std::istringstream iss(cmd);
		auto* sp = actor->GetComponent<SpriteRendererComponent>();

		if (sp == nullptr)
			return nullptr;

		std::string command;       // 如 "slidedown"
		float duration = 1.0f;     // 如 1.0
		std::string transition;    // 可选，如 "easein"

		iss >> command;
		iss >> duration;
		iss >> transition;

		if (GameEngineCore::GetCurrentEngine()->GetDialogueSystem()->IsFastForward())
		{
			duration = 0.f;
		}
		else
		{
			H_LOG_INFO("Command: %s, Duration: %f, Transition: %s", command.c_str(), duration, transition.c_str());
		}

		if (command == "scroll_down" || command == "向下滚动")
		{
			auto script = CreateRef<ScrollTransformScript>(ScrollTransformScript::Direction::Down, sp->sprite->GetSize());
			script->SetDuration(duration);
			return script;
		}

		if (command == "scroll_up" || command == "向上滚动")
		{
			auto transform = CreateRef<ScrollTransformScript>(ScrollTransformScript::Direction::Up, sp->sprite->GetSize());
			transform->SetDuration(duration);
			return transform;
		}

		if (command == "scroll_left" || command == "向左滚动")
		{
			auto transform = CreateRef<ScrollTransformScript>(ScrollTransformScript::Direction::Left, sp->sprite->GetSize());
			transform->SetDuration(duration);
			return transform;
		}

		if (command == "scroll_right" || command == "向右滚动")
		{
			auto transform = CreateRef<ScrollTransformScript>(ScrollTransformScript::Direction::Right, sp->sprite->GetSize());
			transform->SetDuration(duration);
			return transform;
		}

		if (command == "fade_in" || command == "淡入")
		{
			auto transform = CreateRef<FadeInOutTransformScript>(FadeInOutTransformScript::Direction::In);
			transform->SetDuration(duration);
			return transform;
		}

		if (command == "fade_out" || command == "淡出")
		{
			auto transform = CreateRef<FadeInOutTransformScript>(FadeInOutTransformScript::Direction::Out);
			transform->SetDuration(duration);
			return transform;
		}

		H_LOG_WARN("Unknown Command: %s", cmd.c_str());

		return nullptr;
	}

	bool SpriteTransformScriptManager::StartSpriteTransformWithCommand(GameActor* actor, const String& cmd)
	{
		auto script = CreateSpriteTransformWithCommand(actor, cmd);

		if (script != nullptr)
		{
			//开启变换
			script->Start();

			return StartSpriteTransform(actor, script);
		}

		return false;
	}

	bool SpriteTransformScriptManager::StartSpriteTransform(GameActor* actor, const Ref<IAnimationScript>& script)
	{
		if (actor == nullptr)
			return false;

		if (script == nullptr)
			return false;

		auto* com = actor->GetComponent<AnimationScriptComponent>();

		if (com == nullptr)
		{
			com = actor->AddComponent<AnimationScriptComponent>();
		}

		com->scripts.push_back(script);

		return true;
	}

	//void SpriteTransformScriptManager::Update(IScene* scene)
	//{
	//	// 更新变换脚本
	//	{
	//		auto view = scene->GetWorld()->view<AnimationScriptComponent>();
	//
	//		view.each([this](AnimationScriptComponent& com) { // flecs::entity argument is optional
	//			for (auto& script : com.scripts)
	//			{
	//				script->OnUpdate(com.GetOwner());
	//			}
	//			});
	//	}
	//}

}
