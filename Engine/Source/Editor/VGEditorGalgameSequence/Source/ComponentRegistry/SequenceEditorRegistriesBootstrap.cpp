/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "ComponentRegistry/SequenceEditorRegistriesBootstrap.h"

#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "Inspector/Builtin/LegacyDrawerInspectorAdapter.h"
#include "Inspector/SequenceInspectorRegistry.h"
#include "Sequence/EntryUIData.h"

#include <VGImgui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h>

#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

#include <memory>
#include <string>
#include <vector>

namespace VisionGal::Editor
{
	void BootstrapSequenceComponentRegistry(SequenceComponentRegistry& registry)
	{
		std::vector<std::string> types;
		VisionGal::IVGSSequenceComponentManager::Get().EnumerateRegisteredTypeNameIDs(types);

		int priority = 0;
		for (const std::string& id : types)
		{
			SequenceComponentMetadata m;
			m.TypeNameID = id;
			const SequenceEntryUIData ui = SequenceEntryUIDataManager::GetDataByTypeNameID(id);
			m.Category = ui.Category.empty() ? std::string(u8"序列组件") : ui.Category;
			m.Icon = ui.IconLabel.empty() ? std::string(ICON_FA_CUBE) : ui.IconLabel;
			m.Priority = priority++;
			registry.Register(std::move(m));
		}
	}

	void BootstrapSequenceInspectorRegistry(SequenceInspectorRegistry& inspectors, const SequenceComponentRegistry& components)
	{
		for (const SequenceComponentMetadata& meta : components.EnumerateOrdered())
		{
			std::string display = meta.DisplayName;
			if (display.empty())
			{
				const SequenceEntryUIData ui = SequenceEntryUIDataManager::GetDataByTypeNameID(meta.TypeNameID);
				display = ui.Label.empty() ? meta.TypeNameID : ui.Label;
			}
			inspectors.Register(std::make_unique<LegacyDrawerInspectorAdapter>(meta.TypeNameID, std::move(display)));
		}
	}
}
