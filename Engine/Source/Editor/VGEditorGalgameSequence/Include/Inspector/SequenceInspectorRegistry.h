/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Inspector/ISequenceInspector.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace VisionGal
{
	class IVGSSequenceComponent;
}

namespace VisionGal::Editor
{
	class SequenceInspectorRegistry
	{
	public:
		void Register(std::unique_ptr<ISequenceInspector> inspector);

		ISequenceInspector* Lookup(const std::string& typeNameID) const;

		/// Returns false if no inspector or draw failed (null component).
		bool DrawInspector(const std::string& typeNameID, unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context = nullptr);

	private:
		std::unordered_map<std::string, std::unique_ptr<ISequenceInspector>> m_byType;
	};
}
