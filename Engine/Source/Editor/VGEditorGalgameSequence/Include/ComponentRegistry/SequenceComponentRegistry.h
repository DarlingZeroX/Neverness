/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "ComponentRegistry/SequenceComponentMetadata.h"
#include "Sequence/ComponentAdderUI.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace VisionGal::Editor
{
	/// Owns registered component metadata and can build UI category buckets for the palette.
	class SequenceComponentRegistry
	{
	public:
		void Register(SequenceComponentMetadata metadata);
		void Unregister(const std::string& typeNameID);

		const SequenceComponentMetadata* Find(const std::string& typeNameID) const;

		/// Stable ordering: ascending Priority, then TypeNameID.
		std::vector<SequenceComponentMetadata> EnumerateOrdered() const;

		/// Builds palette columns (including empty categories) from default layout + registered types.
		std::vector<CategoryData> BuildPaletteCategories() const;

	private:
		std::unordered_map<std::string, SequenceComponentMetadata> m_byType;
		std::vector<std::string> m_registrationOrder;
	};
}
