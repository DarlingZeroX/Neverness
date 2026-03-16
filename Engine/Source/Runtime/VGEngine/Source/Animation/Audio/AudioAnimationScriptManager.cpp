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

#include "Animation/Audio/AudioAnimationScriptManager.h"
#include "Scene/Components.h"
#include "Animation/Audio/AudioAnimationScript.h"
#include "Animation/Interface/AnimationScriptManager.h"

namespace VisionGal {

	AudioAnimationScriptManager* AudioAnimationScriptManager::GetInstance()
	{
		static AudioAnimationScriptManager s_Manager;
		return &s_Manager;
	}

	Ref<IAnimationScript> AudioAnimationScriptManager::CreateAudioAnimationWithCommand(IGameActor* actor, const String& cmd)
	{
		if (actor == nullptr)
			return nullptr;

		std::istringstream iss(cmd);
		auto* audioSource = actor->GetComponent<AudioSourceComponent>();

		if (audioSource == nullptr)
			return nullptr;

		std::string command;       // 如 "slidedown"
		float duration = 1.0f;     // 如 1.0
		std::string transition;    // 可选，如 "easein"

		iss >> command;
		iss >> duration;
		iss >> transition;

		{
			H_LOG_INFO("Audio Command: %s, Duration: %f, Transition: %s", command.c_str(), duration, transition.c_str());
		}

		if (command == "fade_in" || command == "淡入")
		{
			auto transform = CreateRef<AudioFadeInOutAnimationScript>(AudioFadeInOutAnimationScript::Direction::In);
			transform->SetInVolume(audioSource->volume);
			transform->SetDuration(duration);
			return transform;
		}

		if (command == "fade_out" || command == "淡出")
		{
			auto transform = CreateRef<AudioFadeInOutAnimationScript>(AudioFadeInOutAnimationScript::Direction::Out);
			transform->SetOutVolume(audioSource->volume);
			transform->SetDuration(duration);
			return transform;
		}

		H_LOG_WARN("Unknown Command: %s", cmd.c_str());

		return nullptr;
	}

	bool AudioAnimationScriptManager::StartAudioAnimationWithCommand(IGameActor* actor, const String& cmd)
	{
		auto script = CreateAudioAnimationWithCommand(actor, cmd);

		if (script != nullptr)
		{
			//开启变换
			script->Start();

			//return StartAudioTransform(actor, script);
			return AnimationScriptManager::AddActorAnimationScript(actor, script);
		}

		return false;
	}

	//bool AudioAnimationScriptManager::StartAudioTransform(GameActor* actor, const Ref<IAnimationScript>& script)
	//{
	//	if (actor == nullptr)
	//		return false;
	//
	//	if (script == nullptr)
	//		return false;
	//
	//	auto* com = actor->GetComponent<AnimationScriptComponent>();
	//
	//	if (com == nullptr)
	//	{
	//		com = actor->AddComponent<AnimationScriptComponent>();
	//	}
	//
	//	com->scripts.push_back(script);
	//
	//	return true;
	//}

}
