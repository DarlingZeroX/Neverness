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

#include "Galgame/UISystem/GalUISystem.h"
#include "Core/EventBus.h"
#include "Galgame/Components.h"
#include "UI/UISystem.h"

namespace VisionGal::GalGame
{
	GalGameUISystem::GalGameUISystem()
	{
	}

	void GalGameUISystem::Initialize(const Ref<GalGameContext>& galCtx, IGameEngineContext* context)
	{
		m_GalCtx = galCtx;
		m_GECtx = context;

		EngineEventBus::Get().OnEngineEvent.Subscribe([this](const EngineEvent& evt)
			{
				switch (evt.EventType)
				{
				case EngineEventType::MainSceneChanged:
					m_Scene = evt.Scene;
					break;
				}
			});

		galCtx->uiEventBus.OnUIEvent.Subscribe([this](const GalGameUIEvent& evt)
			{
				switch (evt.EventType)
				{
				case GalGameUIEvent::Type::ShowChoiceUI:
					ShowChoiceUI(evt.ChoiceName, evt.ChoiceOptions);
				}
			});
	}

	void GalGameUISystem::ShowChoiceUI(const std::string& name, const std::vector<std::string>& options)
	{
		m_CurrentChoiceOptions = options;

		auto view = m_Scene->GetWorld()->view<GalGameEngineComponent>();

		std::string choiceUIPath;
		view.each([this, &choiceUIPath](GalGameEngineComponent& com) { // flecs::entity argument is optional
			choiceUIPath = com.choiceUIPath;
			});

		if (choiceUIPath.empty() == false)
		{
			auto doc = UISystem::Get()->LoadUIDocument(choiceUIPath);
			UISystem::Get()->ShowUIDocument(doc.get());
		}
	}

	void GalGameUISystem::ShowFullScreenTextUI(const std::vector<std::string>& texts)
	{
		m_CurrentFullScreenTexts = texts;

		auto view = m_Scene->GetWorld()->view<GalGameEngineComponent>();

		std::string fullScreenTextUIPath;
		view.each([this, &fullScreenTextUIPath](GalGameEngineComponent& com) { // flecs::entity argument is optional
			fullScreenTextUIPath = com.fullScreenTextUIPath;
			});

		if (fullScreenTextUIPath.empty() == false)
		{
			auto doc = UISystem::Get()->LoadUIDocument(fullScreenTextUIPath);
			UISystem::Get()->ShowUIDocument(doc.get());
		}
	}

	std::string GalGameUISystem::GetFullScreenTextItem(int index)
	{
		if (index < m_CurrentFullScreenTexts.size())
			return m_CurrentFullScreenTexts[index];

		return {};
	}

	int GalGameUISystem::GetFullScreenTextSize() const
	{
		return m_CurrentFullScreenTexts.size();
	}

	std::string GalGameUISystem::GetChoiceOptionByIndex(int index)
	{
		if (index < m_CurrentChoiceOptions.size())
			return m_CurrentChoiceOptions[index];

		return {};
	}

	int GalGameUISystem::GetChoiceOptionSize() const
	{
		return m_CurrentChoiceOptions.size();
	}

	void GalGameUISystem::SelectCurrentChoice(int index)
	{
		GalGameUIEvent evt;
		evt.CurrentChoiceIndex = index;
		evt.ChoiceOptions = m_CurrentChoiceOptions;
		evt.EventType = GalGameUIEvent::Type::ChoiceSelected;
		m_GalCtx->uiEventBus.OnUIEvent.Invoke(evt);
	}
}
