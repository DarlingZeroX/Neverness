/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Extensions/SequenceExtensionRegistry.h"

#include "Extensions/ISequenceEditorExtension.h"

namespace VisionGal::Editor
{
	void SequenceExtensionRegistry::Register(std::unique_ptr<ISequenceEditorExtension> ext)
	{
		if (ext == nullptr)
			return;
		m_extensions.push_back(std::move(ext));
	}

	void SequenceExtensionRegistry::NotifySessionBegin()
	{
		for (auto& e : m_extensions)
		{
			if (e)
				e->OnEditorSessionBegin();
		}
	}

	void SequenceExtensionRegistry::NotifySessionEnd()
	{
		for (auto& e : m_extensions)
		{
			if (e)
				e->OnEditorSessionEnd();
		}
	}
}
