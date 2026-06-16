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
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
#include "NNRuntimeCore/Interface/RenderInterface.h"
#include <NNCore/Include/Event/HEventDelegate.h>

namespace NN::Runtime {

	////////////////		Transition Event
	enum class TransitionEventType
	{
		None = 0,
		TransitionCreated,
	};

	struct TransitionEvent
	{
		TransitionEventType EventType = TransitionEventType::None;
		String Layer;
		ISceneTransition* Transition;
	};

	class VG_ENGINE_API TransitionManager
	{
	public:
		static TransitionManager* GetInstance();

		/// 通过转场命令创建转场对象
		static Ref<ISceneTransition> CreateTransitionWithCommand(const String& cmd);
		static Ref<ISceneTransition> CreateCustomImageTransitionWithCommand(const String& imagePath, const String& cmd);

		bool StartTransitionWithCommand(const String& layer, const String& cmd);
		bool StartCustomImageTransitionWithCommand(const String& layer, const String& imagePath, const String& cmd);

		void StartTransition(const Ref<ISceneTransition>& transition);
		void Update();
		ISceneTransition* GetLayerTransition(const String& layer);
		void LayerTransition(const String& layer, const Ref<VGFX::ITexture>& prevFrame, const Ref<VGFX::ITexture>& nextFrame);

		bool IsTransitioning() const;

		/// 中文：立即丢弃所有层上的转场队列（Gal **ResetRuntime** / 紧急切场景用）；不触发转场完成回调。
		void AbortAllTransitions();

		std::string LayerTranslateEnglish(const String& layer);

		NN::Core::HEventDelegate<const TransitionEvent&> OnTransitionEvent;
	private:
		std::unordered_map<String, std::queue<Ref<ISceneTransition>> > m_Transitions;
	};

}