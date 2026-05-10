/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Sequence/ComponentAdderUI.h"
#include "ComponentRegistry/SequenceComponentRegistry.h"

namespace VisionGal::Editor
{
	/// Palette UI with independent render entry; composes `ComponentAdderUI` fed from `SequenceComponentRegistry`.
	class SequenceComponentPaletteWidget
	{
	public:
		SequenceComponentPaletteWidget();

		void ReloadFromRegistry(const SequenceComponentRegistry& registry);

		void Render();

		Horizon::HEventDelegate<std::string> OnComponentChosen;

	private:
		ComponentAdderUI m_adderUI;
	};
}
