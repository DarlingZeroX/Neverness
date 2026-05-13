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

#include "UISystem/GalUISystem.h"
#include "VGCore/Include/Core/EventBus.h"
#include "VGGalgameCore/Include/Components.h"
#include "VGUI/Interface/UISystem.h"

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
					ShowChoiceUI(evt.ChoiceID, evt.ChoiceOptions);
					break;
				case GalGameUIEvent::Type::ShowInputUI:
					ShowInputUI(evt.InputID, evt.InputTitle, evt.InputButtonText);
					break;
				}
			});
	}

	void GalGameUISystem::ShowChoiceUI(const std::string& id, const std::vector<std::string>& options)
	{
		m_CurrentChoiceID = id;
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
		evt.ChoiceID = m_CurrentChoiceID;
		evt.CurrentChoiceIndex = index;
		evt.ChoiceOptions = m_CurrentChoiceOptions;
		evt.EventType = GalGameUIEvent::Type::ChoiceSelected;
		m_GalCtx->uiEventBus.OnUIEvent.Invoke(evt);
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

	void GalGameUISystem::ShowInputUI(const std::string& id, const std::string& title, const std::string& button)
	{
		m_CurrentInputID = id;
		m_CurrentInputTitle = title;
		m_CurrentInputButtonText = button;

		auto view = m_Scene->GetWorld()->view<GalGameEngineComponent>();

		std::string inputUIPath;
		view.each([this, &inputUIPath](GalGameEngineComponent& com) { // flecs::entity argument is optional
			inputUIPath = com.inputUIPath;
			});

		if (inputUIPath.empty() == false)
		{
			auto doc = UISystem::Get()->LoadUIDocument(inputUIPath);
			UISystem::Get()->ShowUIDocument(doc.get());
		}
	}

	std::string GalGameUISystem::GetInputTitle()
	{
		return  m_CurrentInputTitle;
	}

	std::string GalGameUISystem::GetInputButtonText()
	{
		return m_CurrentInputButtonText;
	}

	void GalGameUISystem::InputSubmitted(const std::string& text)
	{
		GalGameUIEvent evt;
		evt.CurrentInputText = text;
		evt.InputID = m_CurrentInputID;
		evt.EventType = GalGameUIEvent::Type::InputSubmitted;
		m_GalCtx->uiEventBus.OnUIEvent.Invoke(evt);
	}

}
