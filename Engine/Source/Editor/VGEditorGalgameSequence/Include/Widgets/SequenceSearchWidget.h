/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <string>

namespace VisionGal::Editor
{
	struct SequenceEditorContext;

	/// Phase-2 placeholder search bar; filter string is exposed for future list filtering.
	class SequenceSearchWidget
	{
	public:
		std::string& GetFilter() { return m_filter; }

		void Render(SequenceEditorContext& ctx);

	private:
		std::string m_filter;
	};
}
