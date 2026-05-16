/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <memory>
#include <vector>
#include "ISequenceEditorExtension.h"

namespace VisionGal::Editor
{
	//class ISequenceEditorExtension;

	class SequenceExtensionRegistry
	{
	public:
		void Register(std::unique_ptr<ISequenceEditorExtension> ext);
		void NotifySessionBegin();
		void NotifySessionEnd();
		[[nodiscard]] const std::vector<std::unique_ptr<ISequenceEditorExtension>>& GetExtensions() const
		{
			return m_extensions;
		}

	private:
		std::vector<std::unique_ptr<ISequenceEditorExtension>> m_extensions;
	};
}
