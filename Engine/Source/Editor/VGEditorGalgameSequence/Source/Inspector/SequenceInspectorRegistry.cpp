/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Inspector/SequenceInspectorRegistry.h"

#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

#include <utility>

namespace VisionGal::Editor
{
	void SequenceInspectorRegistry::Register(std::unique_ptr<ISequenceInspector> inspector)
	{
		if (inspector == nullptr)
			return;
		const std::string key = inspector->GetBoundTypeNameID();
		m_byType[key] = std::move(inspector);
	}

	ISequenceInspector* SequenceInspectorRegistry::Lookup(const std::string& typeNameID) const
	{
		const auto it = m_byType.find(typeNameID);
		if (it == m_byType.end() || it->second == nullptr)
			return nullptr;
		return it->second.get();
	}

	bool SequenceInspectorRegistry::DrawInspector(const std::string& typeNameID, unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context)
	{
		if (component == nullptr)
			return false;
		ISequenceInspector* inspector = Lookup(typeNameID);
		if (inspector == nullptr)
			return false;
		inspector->OnInspectorGUI(index, component, context);
		return true;
	}
}
