/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceComponentPaletteWidget.h"

#include <NNRuntimeImGui/IncludeImGui.h>

namespace VisionGal::Editor
{
	SequenceComponentPaletteWidget::SequenceComponentPaletteWidget()
	{
		m_adderUI.OnIconClicked.Subscribe([this](const std::string& typeNameID) {
			OnComponentChosen.Invoke(typeNameID);
		});
	}

	void SequenceComponentPaletteWidget::ReloadFromRegistry(const SequenceComponentRegistry& registry)
	{
		m_adderUI.SetCategories(registry.BuildPaletteCategories());
	}

	void SequenceComponentPaletteWidget::Render()
	{
		m_adderUI.ShowDemoIconsUI();
	}
}
