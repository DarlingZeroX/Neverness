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

#include "Render/TransitionManager.h"

#include "VGCore/Interface/Loader.h"
#include "Render/Transition.h"

namespace VisionGal {

	TransitionManager* TransitionManager::GetInstance()
	{
		static TransitionManager s_Instance;

		return &s_Instance;
	}

	Ref<ISceneTransition> TransitionManager::CreateTransitionWithCommand(const String& cmd)
	{
		std::istringstream iss(cmd);

		std::string command;       // 如 "fade"
		float duration = 1.0f;     // 如 1.0
		std::string transition;    // 可选，如 "easein"

		iss >> command;
		iss >> duration;
		iss >> transition;

		H_LOG_INFO("Command: %s, Duration: %f, Transition: %s", command.c_str(), duration, transition.c_str());

		if (command == "dissolve" || command == "溶解")
		{
			auto iTransition = MakeRef<DissolveSceneTransition>();
			iTransition->SetDuration(duration);
			return iTransition;
		}

		if (command == "fadeIn" || command == "淡入")
		{
			auto iTransition = MakeRef<FadeSceneTransition>(FadeSceneTransition::FadeType::FadeIn);
			iTransition->SetDuration(duration);
			return iTransition;
		}

		if (command == "fadeOut" || command == "淡出")
		{
			auto iTransition = MakeRef<FadeSceneTransition>(FadeSceneTransition::FadeType::FadeOut);
			iTransition->SetDuration(duration);
			return iTransition;
		}

		if (command == "pushLeft" || command == "向左推入")
		{
			auto iTransition = MakeRef<PushSceneTransition>(PushSceneTransition::PushType::PushLeft);
			iTransition->SetDuration(duration);
			return iTransition;
		}

		if (command == "pushRight" || command == "向右推入")
		{
			auto iTransition = MakeRef<PushSceneTransition>(PushSceneTransition::PushType::PushRight);
			iTransition->SetDuration(duration);
			return iTransition;
		}

		if (command == "pushDown" || command == "向下推入")
		{
			auto iTransition = MakeRef<PushSceneTransition>(PushSceneTransition::PushType::PushDown);
			iTransition->SetDuration(duration);
			return iTransition;
		}

		if (command == "pushUp" || command == "向上推入")
		{
			auto iTransition = MakeRef<PushSceneTransition>(PushSceneTransition::PushType::PushUp);
			iTransition->SetDuration(duration);
			return iTransition;
		}

		return nullptr;
	}

	bool TransitionManager::StartTransitionWithCommand(const String& layer, const String& cmd)
	{
		auto enLayer = LayerTranslateEnglish(layer);
		auto transition = CreateTransitionWithCommand(cmd);

		if (transition != nullptr)
		{
			transition->SetLayer(enLayer);
			transition->Start();
			StartTransition(transition);

			return true;
		}

		return false;
	}

	Ref<ISceneTransition> TransitionManager::CreateCustomImageTransitionWithCommand(const String& imagePath,
		const String& cmd)
	{
		std::istringstream iss(cmd);

		float duration = 1.0f;     // 如 1.0
		std::string transition;    // 可选，如 "easein"

		iss >> duration;
		iss >> transition;

		H_LOG_INFO("Custom Image Transition: %s, Duration: %f, Transition: %s", imagePath.c_str(), duration,
			transition.c_str());

		// 读取自定义转场图片资产
		auto tex = LoadObject<Texture2D>(imagePath);
		if (tex == nullptr)
		{
			H_LOG_WARN("加载自定义转场图片失败: %s", imagePath.c_str());
			return nullptr;
		}

		auto iTransition = MakeRef<CustomImageSceneTransition>(tex);
		iTransition->SetDuration(duration);
		return iTransition;
	}

	bool TransitionManager::StartCustomImageTransitionWithCommand(const String& layer, const String& imagePath,
		const String& cmd)
	{
		auto enLayer = LayerTranslateEnglish(layer);

		auto transition = CreateCustomImageTransitionWithCommand(imagePath, cmd);
		if (transition != nullptr)
		{
			transition->SetLayer(enLayer);
			transition->Start();
			StartTransition(transition);
			return true;
		}
		return false;
	}

	void TransitionManager::StartTransition(const Ref<ISceneTransition>& transition)
	{
		if (transition == nullptr)
			return;

		H_LOG_INFO("Transition[Layer: %s] Start!!!", transition->GetLayer().c_str());

		// 如果当前层有转场在进行，则先结束当前转场
		if (m_Transitions[transition->GetLayer()].empty() == false)
		{
			transition->Finish();
		}

		//m_Transitions.push_back(transition);
		m_Transitions[transition->GetLayer()].push(transition);

		// 转场创建事件
		TransitionEvent evt;
		evt.EventType = TransitionEventType::TransitionCreated;
		evt.Layer = transition->GetLayer();
		evt.Transition = transition.get();
		OnTransitionEvent.Invoke(evt);
	}

	void TransitionManager::Update()
	{
		// 删除完成的场景转换
		//{
		//	std::vector<String> finishLayers;
		//
		//	for (auto& pair: m_Transitions)
		//	{
		//		auto& transition = pair.second;
		//		if (transition == nullptr)
		//			continue;
		//
		//		if (transition->IsFinish())
		//		{
		//			finishLayers.push_back(transition->GetLayer());
		//		}
		//	}
		//
		//	for (auto& layer: finishLayers)
		//	{
		//		m_Transitions.erase(layer);
		//	}
		//}
	}

	ISceneTransition* TransitionManager::GetLayerTransition(const String& layer)
	{
		auto result = m_Transitions.find(layer);
		if (result != m_Transitions.end())
		{
			if (!result->second.empty())
			{
				return result->second.front().get();
			}
		}

		return nullptr;
	}

	void TransitionManager::LayerTransition(const String& layer, const Ref<VGFX::ITexture>& prevFrame,
		const Ref<VGFX::ITexture>& nextFrame)
	{
		auto* transition = GetInstance()->GetLayerTransition(layer);
		if (transition == nullptr)
			return;

		//H_LOG_INFO("Transition[Layer: %s]", layer.c_str());

		transition->SetPrevFrame(prevFrame.get());
		transition->SetNextFrame(nextFrame.get());
		transition->Transition();

		if (transition->IsFinish())
		{
			H_LOG_INFO("Transition[Layer: %s] Finish!!!", layer.c_str());
			//m_Transitions.erase(layer);
			m_Transitions[layer].pop();

			// 如果当前层的转场队列不为空，则继续下一个转场
			if (!m_Transitions[layer].empty())
			{
				auto nextTransition = m_Transitions[layer].front();
				nextTransition->Start();
			}
		}
	}

	bool TransitionManager::IsTransitioning() const
	{
		for (auto& layer: m_Transitions)
		{
			if (layer.second.empty() == false)
				return true;
		}

		return false;
	}

	void TransitionManager::AbortAllTransitions()
	{
		m_Transitions.clear();
	}

	std::string TransitionManager::LayerTranslateEnglish(const String& layer)
	{
		if (layer == "背景")
		{
			return "Background";
		}
		else if (layer == "前景")
		{
			return "Foreground";
		}
		else if (layer == "屏幕")
		{
			return "Screen";
		}
		return layer;
	}
}
