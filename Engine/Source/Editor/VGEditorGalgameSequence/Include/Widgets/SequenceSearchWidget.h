/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "ViewModels/SequenceSearchViewModel.h"

namespace VisionGal::Editor
{
	struct SequenceEditorContext;

	/// Search bar + dimension flags; `SequenceDocumentViewModel` consumes `GetSearchViewModel()`.
	/// 搜索栏与维度开关；`SequenceDocumentViewModel` 消费 `GetSearchViewModel()`。
	class SequenceSearchWidget
	{
	public:
		std::string& GetFilter() { return m_searchViewModel.TextFilter(); }
		SequenceSearchViewModel& GetSearchViewModel() { return m_searchViewModel; }

		void Render(SequenceEditorContext& ctx);

	private:
		SequenceSearchViewModel m_searchViewModel;
	};
}
