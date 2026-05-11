/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "ComponentRegistry/SequenceEditorRegistriesBootstrap.h"

#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "Inspector/BuiltinSequenceInspectors.h"
#include "Inspector/SequenceInspectorRegistry.h"

#include <VGImgui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h>

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

#include <memory>
#include <string>
#include <vector>

namespace VisionGal::Editor
{
	namespace
	{
		void FillPresentationForTypeNameID(const std::string& id, SequenceComponentMetadata& m)
		{
			if (id == VGSSC_CommonDialogue::StaticGetTypeNameID())
			{
				m.DisplayName = u8"普通对话";
				m.Icon = ICON_FA_COMMENT_ALT;
				m.Category = u8"常规演出";
				return;
			}
			if (id == VGSSC_ChangeFigure::StaticGetTypeNameID())
			{
				m.DisplayName = u8"切换立绘";
				m.Icon = ICON_FA_USER;
				m.Category = u8"常规演出";
				return;
			}
			if (id == VGSSC_ChangeBackground::StaticGetTypeNameID())
			{
				m.DisplayName = u8"切换背景";
				m.Icon = ICON_FA_IMAGES;
				m.Category = u8"常规演出";
				return;
			}
			m.DisplayName = id;
			m.Category = u8"序列组件";
			m.Icon = ICON_FA_CUBE;
		}
	}

	void BootstrapSequenceComponentRegistry(SequenceComponentRegistry& registry)
	{
		std::vector<std::string> types;
		VisionGal::IVGSSequenceComponentManager::Get().EnumerateRegisteredTypeNameIDs(types);

		int priority = 0;
		for (const std::string& id : types)
		{
			SequenceComponentMetadata m;
			m.TypeNameID = id;
			FillPresentationForTypeNameID(id, m);
			m.Priority = priority++;
			registry.Register(std::move(m));
		}
	}

	void BootstrapSequenceInspectorRegistry(SequenceInspectorRegistry& inspectors, const SequenceComponentRegistry& components)
	{
		for (const SequenceComponentMetadata& meta : components.EnumerateOrdered())
			inspectors.Register(MakeSequenceInspectorForMetadata(meta));
	}
}
