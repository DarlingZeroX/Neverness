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

#include "Galgame/SpriteAnimationScriptManager.h"
#include "Galgame/GameEngineCore.h"
#include "Galgame/SpriteAnimationScript.h"
#include "HCore/Include/Core/HLocalization.h"
#include "HCore/Include/Core/HStringTools.h"
#include "Scene/Components.h"
#include "Animation/Core/SpriteAnimationScript.h"
#include "Animation/Interface/AnimationScriptManager.h"

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
			auto transform = CreateRef<SpriteFadeInOutTransformScript>(actor, SpriteFadeInOutTransformScript::Direction::In);
			transform->SetDuration(duration);
			return transform;
		}

		if (command == "fade_out" || command == "淡出")
		{
			auto transform = CreateRef<SpriteFadeInOutTransformScript>(actor, SpriteFadeInOutTransformScript::Direction::Out);
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

			return AnimationScriptManager::AddActorAnimationScript(actor, script);
			//return StartSpriteTransform(actor, script);
		}

		return false;
	}

}
